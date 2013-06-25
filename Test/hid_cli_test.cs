//
// hid_cli_test.cs
//
// Author:
//       SjB <steve@sagacity.ca>
//
//

using System;

using SjB.Hid;

namespace SjB.Hid.Test
{
	public static class HidCliTest
	{
		const int VID = 0x0d8c;
		const int PID = 0x000c;

		static void Main(string[] args)
		{
			var dev = new HidDevice(VID, PID);
			dev.Open();


			byte[] d = new byte[] {0x0, 0x0, 0xf, 0x0};
			for (int i = 0; i < 10; i++) {
				d[2] = (byte)(0xf | i);
				int bytesWritten = dev.Write(d);
				Console.WriteLine("Wrote to HID device {0}", bytesWritten);
			}
			dev.Close();
		}
	}

}

