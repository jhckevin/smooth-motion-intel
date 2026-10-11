using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using SmoothMotionDesktop;
class TargetPicker991Tests {
 static int checks;static readonly BindingFlags F=BindingFlags.Instance|BindingFlags.NonPublic;
 static object Get(object w,string n){return w.GetType().GetField(n,F).GetValue(w);}
 static void Set(object w,string n,object v){w.GetType().GetField(n,F).SetValue(w,v);}
 static void Call(object w,string n,params object[] args){w.GetType().GetMethod(n,F).Invoke(w,args);}
 static void Check(bool b,string m){checks++;if(!b)throw new Exception(m);}
 static WindowTarget Target(int i,string title){return new WindowTarget{Handle=new IntPtr(i),ProcessId=i+100,ProcessCreationTimeUtcFileTime=i+1000,IdentityKnown=true,Title=title,ExecutablePath="C:\\Games\\ForzaHorizon6.exe",ClientWidth=1920,ClientHeight=1080,IsVisible=true,IsSupported=true,UnsupportedReason=""};}
 [STAThread]static int Main(){
  RenderOptions.ProcessRenderMode=System.Windows.Interop.RenderMode.SoftwareOnly;
  var app=new Application{ShutdownMode=ShutdownMode.OnExplicitShutdown};
  using(var r=Assembly.GetExecutingAssembly().GetManifestResourceStream("Theme988.xaml"))app.Resources.MergedDictionaries.Add((ResourceDictionary)XamlReader.Load(r));
  DesktopWindow w=null;
  try{
   w=new DesktopWindow();((DispatcherTimer)Get(w,"timer")).Stop();
   var p=(Profile)Get(w,"profile");p.Filter="Forza Horizon 6";p.AutoSelect=false;
   var game=Target(9,"Forza Horizon 6");var fresh=Target(9,"Forza Horizon 6");fresh.ClientWidth=914;fresh.ClientHeight=514;
   var many=Enumerable.Range(20,40).Select(i=>Target(i,"A tool "+i+new string('x',160))).ToList();many.Add(game);
   Set(w,"selectedTarget",null);Set(w,"targetChoiceExplicit",false);Call(w,"RefreshTargetsFrom991",many);
   var box=(ComboBox)Get(w,"targetBox");
   Check(box.Items.Count==42,"all targets kept plus foreground choice");Check(box.SelectedIndex==1&&Object.ReferenceEquals(box.SelectedItem,game),"matching game ranked and initially selected");
   Check(!((EngineHost)Get(w,"engine")).HasProcess,"selection does not start engine");
   Check(TargetPicker991.MatchesFilter("forzahorizon6.exe",game),"case-insensitive executable match");Check(TargetPicker991.MatchesFilter("other;Forza*",game),"wildcard multi-filter match");
   Check(!TargetPicker991.MatchesFilter("Forza Horizon 60",game),"exact nonmatch");
   box.SelectedIndex=0;Call(w,"RefreshTargetsFrom991",many);Check(box.SelectedIndex==0,"explicit foreground preference preserved");
   box.SelectedItem=game;many.Remove(game);many.Add(fresh);Call(w,"RefreshTargetsFrom991",many);
   Check(Object.ReferenceEquals(box.SelectedItem,fresh)&&((WindowTarget)Get(w,"selectedTarget")).ClientWidth==914,"fresh dimensions and identity preserved");
   fresh.ProcessCreationTimeUtcFileTime++;Call(w,"RefreshTargetsFrom991",new List<WindowTarget>{game});Check(box.SelectedIndex==0,"recycled handle never silently retargeted");
   var twin=Target(10,"Forza Horizon 6");Check(TargetPicker991.Choose(new List<WindowTarget>{game,twin},null,false,p.Filter)==null,"ambiguous profile never guessed");
   game.IsSupported=false;Check(TargetPicker991.Choose(new List<WindowTarget>{game},null,false,p.Filter)==null,"unsupported match not auto-selected");game.IsSupported=true;
   Call(w,"RefreshTargetsFrom991",new List<WindowTarget>());Check(box.Items.Count==1,"empty list keeps foreground choice");
   typeof(ComboBox).GetMethod("OnDropDownOpened",BindingFlags.Instance|BindingFlags.NonPublic).Invoke(box,new object[]{EventArgs.Empty});Check(box.Items.Count>1,"opening refreshes current OS catalog");
   Set(w,"selectedTarget",null);Set(w,"targetChoiceExplicit",false);Call(w,"RefreshTargetsFrom991",many);var before=box.Items.Count;Set(w,"running",true);Call(w,"RefreshTargetsFrom991",new List<WindowTarget>());Check(box.Items.Count==before,"running capture freezes target");Set(w,"running",false);
   box.ItemTemplate.Seal();var item=(TextBlock)box.ItemTemplate.LoadContent();item.DataContext=game;item.Measure(new Size(370,60));item.Arrange(new Rect(0,0,370,40));item.UpdateLayout();
   Check(item.Text.Contains("Forza Horizon 6"),"row text resolves WindowTarget.ToString");Check(item.TextTrimming==TextTrimming.CharacterEllipsis,"long title ellipsis");
   box.ApplyTemplate();var popup=(System.Windows.Controls.Primitives.Popup)box.Template.FindName("PART_Popup",box);Check(popup!=null,"popup template present");
   var border=(Border)popup.Child;var scroll=(ScrollViewer)border.Child;Check(scroll.HorizontalScrollBarVisibility==ScrollBarVisibility.Disabled,"menu horizontal overflow prevented");Check(scroll.VerticalScrollBarVisibility==ScrollBarVisibility.Auto,"long lists scroll");
   Check(System.Windows.Data.BindingOperations.GetBinding(border,FrameworkElement.WidthProperty)!=null,"menu width bound to picker");
   Set(w,"selectedTarget",null);Set(w,"targetChoiceExplicit",false);Call(w,"RefreshTargetsFrom991",new List<WindowTarget>{game});
   var grid=(FrameworkElement)w.Content;grid.Measure(new Size(1180,760));grid.Arrange(new Rect(0,0,1180,760));grid.UpdateLayout();
   Directory.CreateDirectory("tests/visual991");var bmp=new RenderTargetBitmap(2065,1330,168,168,PixelFormats.Pbgra32);bmp.Render(grid);var enc=new PngBitmapEncoder();enc.Frames.Add(BitmapFrame.Create(bmp));using(var o=File.Create("tests/visual991/picker-175.png"))enc.Save(o);
   File.WriteAllText("tests/picker991-tests.json","{\"pass\":true,\"checks\":"+checks+",\"engineStarted\":false,\"testMode\":\"remote WPF software and selection events\"}");Console.WriteLine("PASS "+checks+" picker behavior checks; no capture/GPU.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
  finally{if(w!=null){Set(w,"allowClose",true);w.Close();}app.Shutdown();}
 }
}