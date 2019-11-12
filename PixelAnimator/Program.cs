using System;
using System.Drawing;
using System.IO;

namespace PixelAnimator
{
    /// <summary>
    /// PixelAnimator is a simple command-line utility for converting
    /// image files to uncompressed RGB data for display in the Player
    /// Arduino sketch. 
    /// </summary>
    class PixelAnimator
    {
        static int Main(string[] args)
        {
            try
            {
                if (args.Length != 4)
                    throw new Exception("Invalid cmd line params. Usage:\nPixelAnimator repeatcount frametime(mS) inputfilepath outputfilepath");

                var repeatCount = Convert.ToInt16(args[0]);
                var frameTime = Convert.ToInt32(args[1]);
                var inputFile = args[2];
                var outputFile = args[3];

                if (repeatCount > 255 || repeatCount < 1 )
                    throw new Exception($"Invalid repeat count value '{repeatCount}', value must be 1-255");
                if (frameTime > 255 || frameTime < 0)
                    throw new Exception($"Invalid frametime value '{frameTime}', value must be 0-255");
                if (!File.Exists(inputFile))
                    throw new Exception($"Input file '{inputFile}' not found");
                if (File.Exists(outputFile))
                    Console.WriteLine($"overwriting output file '{outputFile}'");

                Console.WriteLine($"repeatcount: {repeatCount}");
                Console.WriteLine($"frametime: {frameTime}");
                Console.WriteLine($"inputfile: {inputFile}");
                Console.WriteLine($"outputfile: {outputFile}");

                var bmp = new Bitmap(inputFile);
                var pm = ImageToPixelmap(bmp, (byte)frameTime, (byte)repeatCount);
                var pmf = new FileStream(outputFile, FileMode.Create);
                pmf.Write(pm, 0, pm.Length);

                Console.WriteLine($"output file size: {pm.Length}");
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return 1;
            }

            return 0;
        }

        // Read the pixels of the specified image to build a pixelmap
        private static byte[] ImageToPixelmap(Bitmap bmp, byte mSPerFrame, byte repeatCount)
        {
            const int cHeaderSize = 4;

            var pxm = new byte[cHeaderSize + (bmp.Width * bmp.Height * 3)];
            var o = 0;

            // Width
            var wb = BitConverter.GetBytes(Convert.ToUInt16(bmp.Width));
            if (BitConverter.IsLittleEndian)
            {
                pxm[o++] = wb[0];
                pxm[o++] = wb[1];
            }
            else
            {
                pxm[o++] = wb[1];
                pxm[o++] = wb[0];
            }

            // mS per frame
            pxm[o++] = mSPerFrame;

            // number of times to repeat the whole file
            pxm[o++] = repeatCount;

            // Write each pixel RGB value to array
            for (int j = 0; j < bmp.Height; j++)
                for (int i = 0; i < bmp.Width; i++)
                {
                    var pixel = bmp.GetPixel(i, j);
                    pxm[o++] = pixel.R;
                    pxm[o++] = pixel.G;
                    pxm[o++] = pixel.B;
                }

            return pxm;
        }
    }
}
