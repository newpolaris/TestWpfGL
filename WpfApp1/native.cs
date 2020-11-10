using System;
using System.Runtime.InteropServices;

namespace Native
{

    public class WGLContext
    {
        private const string DLL_PATH = @"WGLContext.dll";

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GLCreate(IntPtr hWND);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        public static extern int Render();
    }
}