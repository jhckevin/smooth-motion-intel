using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using SmoothMotionDesktop;
class VisualQa989 {
 static void ScrollEnd(DependencyObject v){var sv=v as ScrollViewer;if(sv!=null)sv.ScrollToEnd();for(int i=0;i<VisualTreeHelper.GetChildrenCount(v);i++)ScrollEnd(VisualTreeHelper.GetChild(v,i));}
 static readonly BindingFlags F=BindingFlags.Instance|BindingFlags.NonPublic;
 static void Call(object w,string n,params object[] args){w.GetType().GetMethod(n,F).Invoke(w,args);}
 static void Write(UIElement visual,int w,int h,double scale,string path){var bmp=new RenderTargetBitmap((int)Math.Ceiling(w*scale),(int)Math.Ceiling(h*scale),96*scale,96*scale,PixelFormats.Pbgra32);bmp.Render(visual);var encoder=new PngBitmapEncoder();encoder.Frames.Add(BitmapFrame.Create(bmp));using(var o=File.Create(path))encoder.Save(o);var jpeg=new JpegBitmapEncoder{QualityLevel=80};jpeg.Frames.Add(BitmapFrame.Create(bmp));using(var o=File.Create(Path.ChangeExtension(path,"jpg")))jpeg.Save(o);}
 [STAThread]static int Main(){
  RenderOptions.ProcessRenderMode=System.Windows.Interop.RenderMode.SoftwareOnly;
  string root=AppDomain.CurrentDomain.BaseDirectory;Directory.CreateDirectory(Path.Combine(root,"tests","visual993"));Directory.CreateDirectory(Path.Combine(root,"assets"));
  var app=new Application{ShutdownMode=ShutdownMode.OnExplicitShutdown};
  using(var r=Assembly.GetExecutingAssembly().GetManifestResourceStream("Theme988.xaml"))app.Resources.MergedDictionaries.Add((ResourceDictionary)XamlReader.Load(r));
  var window=new DesktopWindow();((DispatcherTimer)window.GetType().GetField("timer",F).GetValue(window)).Stop();
  try{
   foreach(var size in new[]{16,24,32,48,64,256}){
    var tile=new Border{Width=size,Height=size,CornerRadius=new CornerRadius(size*.2),Background=(SolidColorBrush)new BrushConverter().ConvertFromString("#20262D"),Child=VectorIcons989.Create("logo",size*.79)};
    tile.Measure(new Size(size,size));tile.Arrange(new Rect(0,0,size,size));tile.UpdateLayout();Write(tile,size,size,1,Path.Combine(root,"assets","icon-"+size+".png"));
   }
   var grid=(FrameworkElement)window.Content;
   foreach(double scale in new[]{1.0,1.25,1.5,1.75,2.0}){
    Call(window,"ShowMain");grid.Measure(new Size(1180,760));grid.Arrange(new Rect(0,0,1180,760));grid.UpdateLayout();Write(grid,1180,760,scale,Path.Combine(root,"tests","visual993","main-"+(int)(scale*100)+".png"));
   }
   var profile=(Profile)window.GetType().GetField("profile",F).GetValue(window);profile.OutputMode="fullscreen";
   Call(window,"ShowMain");grid.Measure(new Size(1180,760));grid.Arrange(new Rect(0,0,1180,760));grid.UpdateLayout();Write(grid,1180,760,1.75,Path.Combine(root,"tests","visual993","output-preview-175.png"));profile.OutputMode="follow";
   foreach(string page in new[]{"ShowMain","ShowSettings","ShowManual","ShowProfileEditor"}){
    if(page=="ShowProfileEditor")Call(window,page,false);else Call(window,page);
    grid.Measure(new Size(920,640));grid.Arrange(new Rect(0,0,920,640));grid.UpdateLayout();Write(grid,920,640,1,Path.Combine(root,"tests","visual993","compact-"+page+".png"));
   }
   Call(window,"ShowSettings");grid.Measure(new Size(920,640));grid.Arrange(new Rect(0,0,920,640));grid.UpdateLayout();ScrollEnd(grid);grid.UpdateLayout();Write(grid,920,640,1,Path.Combine(root,"tests","visual993","diagnostics-bottom994.png"));
   File.WriteAllText(Path.Combine(root,"tests","visual993","report.json"),"{\"pass\":true,\"renderMode\":\"SoftwareOnly\",\"scales\":[100,125,150,175,200],\"osDpiChanged\":false,\"windowShown\":false,\"engineStarted\":false,\"iconSizes\":[16,24,32,48,64,256],\"scope\":\"Offscreen vector/text layout; physical monitor validation remains separate\"}");
   Console.WriteLine("PASS offscreen WPF software rendering at 100/125/150/175/200%; no display settings, no capture, no inference.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
  finally{window.GetType().GetField("allowClose",F).SetValue(window,true);window.Close();app.Shutdown();}
 }
}
