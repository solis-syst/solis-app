using System.Runtime.InteropServices;
using System.Text.Json;

internal static class Program
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNameCaseInsensitive = true
    };

    private static void Main()
    {
        string? line;

        while ((line = Console.ReadLine()) != null)
        {
            try
            {
                var cmd = JsonSerializer.Deserialize<MouseCommand>(line, JsonOptions);
                if (cmd == null) continue;

                switch (cmd.Action)
                {
                    case "move":
                        if (cmd.X.HasValue && cmd.Y.HasValue)
                            CursorController.SetCursorPos(cmd.X.Value, cmd.Y.Value);
                        break;

                    case "down":
                        CursorController.MouseDown();
                        break;

                    case "up":
                        CursorController.MouseUp();
                        break;

                    case "click":
                        CursorController.Click();
                        break;

                    case "scroll":
                        if (cmd.DeltaY.HasValue)
                            CursorController.Scroll(cmd.DeltaY.Value);
                        break;
                }
            }
            catch
            {
            }
        }
    }
}

internal sealed record MouseCommand(string Action, int? X, int? Y, int? DeltaY);

internal static class CursorController
{
    private const uint MouseEventfLeftDown = 0x0002;
    private const uint MouseEventfLeftUp = 0x0004;
    private const uint MouseEventfWheel = 0x0800;

    [DllImport("user32.dll")]
    private static extern bool NativeSetCursorPos(int x, int y);

    [DllImport("user32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
    private static extern void mouse_event(uint dwFlags, uint dx, uint dy, uint dwData, uint dwExtraInfo);

    public static void SetCursorPos(int x, int y) => NativeSetCursorPos(x, y);

    public static void MouseDown() => mouse_event(MouseEventfLeftDown, 0, 0, 0, 0);

    public static void MouseUp() => mouse_event(MouseEventfLeftUp, 0, 0, 0, 0);

    public static void Click() => mouse_event(MouseEventfLeftDown | MouseEventfLeftUp, 0, 0, 0, 0);

    public static void Scroll(int deltaY)
    {
        var rawDelta = unchecked((uint)(-deltaY));
        mouse_event(MouseEventfWheel, 0, 0, rawDelta, 0);
    }
}
