//
// HidDevice.cs
//
// Author:
//       SjB <steve@sagacity.ca>
//

using System;
using System.Runtime.InteropServices;

namespace SjB.Hid
{

	internal class SafeHidHandle : SafeHandle
	{
		public SafeHidHandle()
			: base (IntPtr.Zero, true) { }

		public SafeHidHandle(IntPtr handle)
			: base (handle, true) { }

		public override bool IsInvalid
		{
			get { return (handle == IntPtr.Zero || handle.ToInt64() == -1); }
		}

		protected override bool ReleaseHandle()
		{
			bool bSuccess = true;
			if (!IsInvalid) {
				bSuccess = (0 == HidCli.hid_device_close(handle));
				handle = IntPtr.Zero;
			}
			return bSuccess;
		}
	}

	public class HidDevice : IDisposable
	{
		public short PID { get; private set; }
		public short VID { get; private set; }

		SafeHidHandle hdev;

		public HidDevice(short vid, short pid)
		{
			PID = pid;
			VID = vid;
		}

		public void Open()
		{
			hdev = HidCli.hid_device_open(VID, PID);
			if (hdev.IsInvalid)
				throw new InvalidOperationException("Can't open Cavra Attenuator.");
		}

		public void Close()
		{
			hdev.Close();
		}

		public void Dispose()
		{
			hdev.Dispose();
		}

		public int Write(byte[] data)
		{
			IntPtr ptr = Marshal.AllocHGlobal(data.Length);
			Marshal.Copy(data, 0, ptr, data.Length);
			int amount = HidCli.hid_device_write(hdev, ptr, data.Length);
			Marshal.FreeHGlobal(ptr);
			return amount;
		}
	}

	internal static class HidCli
	{

		private const string HID_CLI_DLL = "hid-cli.dll";

		[DllImport(HID_CLI_DLL, SetLastError = true)]
		public static extern SafeHidHandle hid_device_open(short vid, short pid);

		[DllImport(HID_CLI_DLL, SetLastError = true)]
		public static extern int hid_device_close([In] IntPtr hdev);

		[DllImport(HID_CLI_DLL, SetLastError = true)]
		public static extern int hid_device_write([In] SafeHidHandle hdev, [In] IntPtr buffer, int count);
	}
}

