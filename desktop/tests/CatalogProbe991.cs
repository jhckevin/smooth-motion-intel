using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Text;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Web.Script.Serialization;
using SmoothMotionDesktop;
class CatalogProbe991 {
 delegate bool Proc(IntPtr h,IntPtr l);
 [DllImport("user32.dll")] static extern bool EnumWindows(Proc p,IntPtr l);
 [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr h);
 [DllImport("user32.dll")] static extern bool IsIconic(IntPtr h);
 [DllImport("user32.dll")] static extern IntPtr GetWindow(IntPtr h,uint c);
 [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr h,StringBuilder b,int n);
 static int Main(string[] args) {
  if(args.Length!=2)return 2; uint target=UInt32.Parse(args[0]);var rows=new List<object>();
  string listError="";var listed=new List<WindowTarget>();try{listed=WindowCatalog.List(System.Diagnostics.Process.GetCurrentProcess().Id);}catch(Exception e){listError=e.ToString();}
  Proc cb=delegate(IntPtr h,IntPtr unused){uint pid;GetWindowThreadProcessId(h,out pid);if(pid!=target)return true;
   var title=new StringBuilder(1024);GetWindowText(h,title,title.Capacity);WindowTarget t=null;string error="";
   try{t=(WindowTarget)typeof(WindowCatalog).GetMethod("Inspect",BindingFlags.Static|BindingFlags.NonPublic).Invoke(null,new object[]{h});}catch(Exception e){error=e.ToString();}
   rows.Add(new {hwnd=h.ToInt64(),title=title.ToString(),visible=IsWindowVisible(h),minimized=IsIconic(h),owner=GetWindow(h,4).ToInt64(),listed=listed.Any(x=>x.Handle==h),supported=t==null?false:t.IsSupported,reason=t==null?"":t.UnsupportedReason,width=t==null?0:t.ClientWidth,height=t==null?0:t.ClientHeight,identity=t==null?false:t.IdentityKnown,error=error});return true;};
  bool ok=EnumWindows(cb,IntPtr.Zero);File.WriteAllText(args[1],new JavaScriptSerializer().Serialize(new {utc=DateTime.UtcNow,processId=target,enumOk=ok,totalListed=listed.Count,listError=listError,windows=rows}));return ok?0:3;
 }
}