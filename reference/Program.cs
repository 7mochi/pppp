using System;

internal static class Program
{
    private static int Main(string[] args)
    {
        if (args.Length == 0)
            return Usage();

        switch (args[0])
        {
            case "--parity":
                if (args.Length != 1)
                    return Usage();

                Parity.Run();
                return 0;

            case "--mods":
                if (args.Length != 1)
                    return Usage();

                Mods.Run();
                return 0;

            default:
                return Usage();
        }
    }

    private static int Usage()
    {
        Console.Error.WriteLine("usage: reference --parity");
        Console.Error.WriteLine("       reference --mods");
        return 2;
    }
}
