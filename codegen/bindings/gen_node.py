#!/usr/bin/env python3

"""
Turns the C ABI into `node/native.g.js`, the Koffi type objects the binding needs.
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path

import clang.cindex as clang

PROJECT = Path(__file__).resolve().parent.parent.parent
HEADER_PATH = PROJECT / "include" / "pppp" / "capi.h"
NATIVE_OUT = PROJECT / "node" / "native.g.js"

# Declared without a body, so they cross as `void *` and the glue keeps the pointer.
HANDLES = ("pppp_beatmap", "pppp_mods", "pppp_strains", "pppp_timed_difficulty")

# Koffi already calls these the same thing, so the name goes out as it came in.
SAME_NAME = ("double", "float", "size_t")

# These cross under a different name. `pppp_result` is an enum, so it crosses as int32.
NEW_NAME = {
    "pppp_int32": "int32",
    "pppp_uint32": "uint32",
    "pppp_int64": "int64",
    "pppp_result": "int32",
}


@dataclass
class Field:
    name: str
    type_name: str
    array: str | None = None


@dataclass
class Struct:
    name: str
    fields: list[Field] = field(default_factory=list)
    union_fields: list[Field] | None = None


@dataclass
class Function:
    name: str
    return_type: str
    arguments: list[str]


@dataclass
class Header:
    structs: list[Struct]
    functions: list[Function]
    constants: dict[str, int]


def split_type(type_name: str) -> tuple[str, int]:
    """The base type and how many pointers wrap it. Koffi has no `const`."""
    text = " ".join(word for word in type_name.split() if word != "const")
    return text.replace("*", " ").strip(), text.count("*")


def primitive_name(base_type: str) -> str:
    """The Koffi name of a built-in type, or an error naming the type `capi.h` grew."""
    if base_type in SAME_NAME:
        return base_type

    name = NEW_NAME.get(base_type)
    if name is None:
        raise SystemExit(
            f"no Koffi mapping for {base_type!r}; "
            "add it to SAME_NAME or NEW_NAME in codegen/bindings/gen_node.py"
        )
    return name


def value_expression(base_type: str, struct_names: set[str]) -> str:
    """A type read by value: either a struct we declare or a primitive."""
    return f"types.{base_type}" if base_type in struct_names else repr(primitive_name(base_type))


def pointer_expression(base_type: str, struct_names: set[str]) -> str:
    """A type read through a pointer. `char *` is a string; a handle has no type Koffi can name."""
    if base_type == "char":
        return repr("str")

    if base_type in HANDLES or base_type == "void":
        return repr("void *")

    if base_type in struct_names:
        return f"koffi.pointer(types.{base_type})"

    return f"koffi.pointer({repr(primitive_name(base_type))})"


def in_header(cursor: clang.Cursor, path: Path) -> bool:
    source = cursor.location.file
    return source is not None and Path(source.name).resolve() == path


def is_pppp_declaration(cursor: clang.Cursor, kind: clang.CursorKind, path: Path) -> bool:
    """A `pppp_` declaration of `kind` that lives in `capi.h`."""
    return (
        cursor.kind == kind
        and cursor.spelling.startswith("pppp_")
        and in_header(cursor, path)
    )


def has_body(cursor: clang.Cursor) -> bool:
    """Whether the struct declares a field or a union, which is what tells it from a handle."""
    return any(
        child.kind in (clang.CursorKind.FIELD_DECL, clang.CursorKind.UNION_DECL)
        for child in cursor.get_children()
    )


def field_of(cursor: clang.Cursor) -> Field:
    """An array's type spells as `pppp_int32[18]`, which Koffi can't read, so the extent comes
    from the tokens and the element type is all this keeps."""
    tokens = [token.spelling for token in cursor.get_tokens()]
    array = None
    if "[" in tokens:
        opening = tokens.index("[")
        closing = tokens.index("]", opening)
        array = "[" + "".join(tokens[opening + 1:closing]) + "]"
    type_name = cursor.type.element_type.spelling if array else cursor.type.spelling
    return Field(name=cursor.spelling, type_name=type_name, array=array)


def parse_struct(cursor: clang.Cursor) -> Struct:
    """The union stays separate because Koffi declares it as its own type."""
    struct = Struct(name=cursor.spelling)

    for child in cursor.get_children():
        if child.kind == clang.CursorKind.FIELD_DECL:
            struct.fields.append(field_of(child))
        elif child.kind == clang.CursorKind.UNION_DECL:
            struct.union_fields = [
                field_of(member)
                for member in child.get_children()
                if member.kind == clang.CursorKind.FIELD_DECL
            ]

    return struct


def parse_structs(translation_unit: clang.TranslationUnit, path: Path) -> list[Struct]:
    """The structs that declare fields, so a handle gets skipped."""
    return [
        parse_struct(cursor)
        for cursor in translation_unit.cursor.get_children()
        if is_pppp_declaration(cursor, clang.CursorKind.STRUCT_DECL, path) and has_body(cursor)
    ]


def parse_functions(translation_unit: clang.TranslationUnit, path: Path) -> list[Function]:
    functions = []
    for cursor in translation_unit.cursor.get_children():
        if not is_pppp_declaration(cursor, clang.CursorKind.FUNCTION_DECL, path):
            continue

        functions.append(
            Function(
                name=cursor.spelling,
                return_type=cursor.result_type.spelling,
                arguments=[
                    argument.type.spelling
                    for argument in cursor.get_arguments()
                ],
            )
        )
    return functions


def parse_constants(text: str) -> dict[str, int]:
    """The numeric `#define`s, read as text because libclang resolves a macro away."""
    return {
        match.group(1): int(match.group(2))
        for match in re.finditer(r"^#define\s+(PPPP_[A-Z0-9_]+)\s+([0-9]+)\s*$", text, re.M)
    }


def parse(path: Path = HEADER_PATH) -> Header:
    """Reads the header twice on purpose: `parse_constants` needs the source text because libclang
    expands a macro away, and everything else comes from the translation unit.
    """
    text = path.read_text(encoding="utf-8")
    translation_unit = clang.Index.create().parse(str(path), args=["-std=c99"])

    errors = [
        diagnostic.spelling
        for diagnostic in translation_unit.diagnostics
        if diagnostic.severity >= clang.Diagnostic.Error
    ]
    if errors:
        raise SystemExit("could not parse %s: %s" % (path, "; ".join(errors)))

    header_path = Path(path).resolve()
    return Header(
        structs=parse_structs(translation_unit, header_path),
        functions=parse_functions(translation_unit, header_path),
        constants=parse_constants(text),
    )


def union_name(struct: Struct) -> str:
    return f"{struct.name}_union"


def field_expression(field: Field, struct_names: set[str], constants: dict[str, int]) -> str:
    """The Koffi type of one field, with the extent of an array already resolved to its number."""
    base_type, pointers = split_type(field.type_name)
    if field.array:
        length = constants[field.array[1:-1]]
        return f"koffi.array({value_expression(base_type, struct_names)}, {length})"

    if base_type in HANDLES:
        return repr("void *")

    return (
        pointer_expression(base_type, struct_names)
        if pointers
        else value_expression(base_type, struct_names)
    )


def field_lines(members: list[Field], struct_names: set[str], constants: dict[str, int]) -> str:
    """The `name: type,` lines of a struct body."""
    return "\n".join(
        f"  {member.name}: {field_expression(member, struct_names, constants)},"
        for member in members
    )


def render_struct(struct: Struct, struct_names: set[str], constants: dict[str, int]) -> str:
    """The union goes out before the struct that uses it, because the entry has to exist by the
    time the field list refers to it.
    """
    blocks = []
    if struct.union_fields:
        blocks.append(
            f"types.{union_name(struct)} = koffi.union({union_name(struct)!r}, {{\n"
            f"{field_lines(struct.union_fields, struct_names, constants)}\n"
            "});"
        )
    union = f"\n  union: types.{union_name(struct)}," if struct.union_fields else ""
    blocks.append(
        f"types.{struct.name} = koffi.struct({struct.name!r}, {{\n"
        f"{field_lines(struct.fields, struct_names, constants)}{union}\n"
        "});"
    )
    return "\n\n".join(blocks)


def argument_expression(type_name: str, struct_names: set[str]) -> str:
    """The Koffi type of one argument. A non-const pointer is an out parameter the call writes
    through, and a double pointer holds the address of an array the call owns."""
    base_type, pointers = split_type(type_name)
    if base_type == "void":
        return pointer_expression(base_type, struct_names)

    if pointers >= 2:
        return "koffi.out(koffi.pointer('void *'))"

    if pointers == 1 and not type_name.startswith("const "):
        if base_type in HANDLES:
            return "koffi.out(koffi.pointer('void *'))"
        if base_type in struct_names:
            return f"koffi.out(koffi.pointer(types.{base_type}))"
        return f"koffi.out(koffi.pointer({repr(primitive_name(base_type))}))"

    return (
        pointer_expression(base_type, struct_names)
        if pointers
        else value_expression(base_type, struct_names)
    )


def result_expression(type_name: str, struct_names: set[str]) -> str:
    """Only a bare `void` is special. A `void *` returns through the normal pointer path."""
    base_type, pointers = split_type(type_name)
    if base_type == "void" and pointers == 0:
        return repr("void")
    return (
        pointer_expression(base_type, struct_names)
        if pointers
        else value_expression(base_type, struct_names)
    )


def declare_lines(header: Header, struct_names: set[str]) -> str:
    """The body of `declare`, one `lib.func` per function."""
    return "\n".join(
        f"  fn.{function.name} = lib.func("
        f"{function.name!r}, {result_expression(function.return_type, struct_names)}, "
        "["
        + ", ".join(argument_expression(t, struct_names) for t in function.arguments)
        + "]);"
        for function in header.functions
    )


def render_native(header: Header) -> str:
    struct_names = {struct.name for struct in header.structs}
    blocks = ['import * as koffi from "koffi";\n\nconst types = {};']
    blocks += [render_struct(struct, struct_names, header.constants) for struct in header.structs]
    blocks.append(
        "function declare(lib) {\n"
        "  const fn = {};\n"
        f"{declare_lines(header, struct_names)}\n"
        "  return fn;\n"
        "}"
    )
    blocks.append("export { declare, types };")
    return "\n\n".join(blocks) + "\n"


if __name__ == "__main__":
    NATIVE_OUT.write_text(render_native(parse()), encoding="utf-8")
    print(f"wrote {NATIVE_OUT.name}")
