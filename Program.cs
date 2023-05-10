using System.Drawing;
using System.Drawing.Imaging;
using System.Net.Sockets;
using System.Runtime.InteropServices;

const string DllPath = "get_screen_size.dll";

[DllImport(DllPath , CallingConvention = CallingConvention.Cdecl)]
extern static void get_screen_size(out int width, out int height);

ImageCodecInfo? GetEncoder(ImageFormat format) {
    ImageCodecInfo[] codecs = ImageCodecInfo.GetImageDecoders();
    foreach (ImageCodecInfo codec in codecs)
    {
        if (codec.FormatID == format.Guid)
        {
            return codec;
        }
    }
    return null;
}

const long FPS = 2;

int[] screen_size = new int[2];

get_screen_size(out screen_size[0], out screen_size[1]);

byte[] screen_size_bytes = new byte[screen_size.Length * sizeof(int)];
Buffer.BlockCopy(screen_size, 0, screen_size_bytes, 0, screen_size_bytes.Length);

int width = screen_size[0];
int height = screen_size[1];

byte[] on_connect_message = new byte[] { 1 };

for (;;) {
    Thread.Sleep(5000);
    Socket client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
    try {
        client.Connect("192.168.1.100", 5000);
        client.Send(on_connect_message);
        client.Send(screen_size_bytes);
    } catch (SocketException e) {
        Console.WriteLine(e.Message);
        client.Close();
        continue;
    }

    for (;;) {
        long start = DateTimeOffset.Now.ToUnixTimeMilliseconds();
        Bitmap bmp = new Bitmap(width, height, PixelFormat.Format32bppArgb);
        Graphics gfx = Graphics.FromImage(bmp);
        gfx.CopyFromScreen(0, 0, 0, 0, new Size(width, height), CopyPixelOperation.SourceCopy);
        Bitmap resized_bmp = new Bitmap(bmp, new Size(width/2, height/2));

        MemoryStream s = new MemoryStream();

        EncoderParameters p = new EncoderParameters(1);
        p.Param[0] = new EncoderParameter(Encoder.Quality, 70L);

        ImageCodecInfo? encoder = GetEncoder(ImageFormat.Jpeg);

        if (encoder == null) return;

        resized_bmp.Save(s, encoder, p);

        byte[] data = s.ToArray();
        try {
            client.Send(data);
        } catch (SocketException e) {
            Console.WriteLine(e.Message);
            client.Close();
            break;
        }
        long end = DateTimeOffset.Now.ToUnixTimeMilliseconds();
        long time_2_sleep = 1000L/FPS - (end - start);
        if (time_2_sleep > 0) Thread.Sleep(TimeSpan.FromMilliseconds(time_2_sleep));
    }
}