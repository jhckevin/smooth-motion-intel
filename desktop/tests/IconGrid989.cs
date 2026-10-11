using System;
using System.IO;
using System.Text;
using System.Linq;
using System.Globalization;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Security.Cryptography;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using SmoothMotionDesktop;
class IconGrid989 {
 static CultureInfo C=CultureInfo.InvariantCulture;
 static string N(double n){return n.ToString("0.####",C);}
 static string Esc(string s){return System.Security.SecurityElement.Escape(s);}
 static string Color(Brush b){var s=b as SolidColorBrush;return s==null?"none":String.Format("#{0:X2}{1:X2}{2:X2}",s.Color.R,s.Color.G,s.Color.B);}
 static Brush B(string s){return (Brush)new BrushConverter().ConvertFromString(s);}
 static TextBlock Text(string s,double size,string color){return new TextBlock{Text=s,FontSize=size,Foreground=B(color),FontFamily=new FontFamily("Segoe UI")};}
 static void Place(Canvas c,UIElement el,double x,double y){Canvas.SetLeft(el,x);Canvas.SetTop(el,y);c.Children.Add(el);}
 static string Hash(string path){using(var sha=SHA256.Create())using(var f=File.OpenRead(path))return BitConverter.ToString(sha.ComputeHash(f)).Replace("-","").ToLowerInvariant();}
 static void SvgIcon(StringBuilder b,string name,double x,double y,double size){
  var box=(Viewbox)VectorIcons989.Create(name,size);var canvas=(Canvas)box.Child;
  b.AppendFormat(C,"<g transform=\"translate({0} {1}) scale({2})\">\n",N(x),N(y),N(size/24));
  foreach(UIElement el in canvas.Children){
   var shape=el as Shape;if(shape==null)throw new InvalidOperationException("Unsupported icon element");
   var m=shape.RenderTransform.Value;string transform=m.IsIdentity?"":String.Format(C," transform=\"matrix({0} {1} {2} {3} {4} {5})\"",N(m.M11),N(m.M12),N(m.M21),N(m.M22),N(m.OffsetX),N(m.OffsetY));
   var path=shape as System.Windows.Shapes.Path;
   if(path!=null){
    string data=Regex.Replace(path.Data.ToString(C),@"^F[01]\s*","");
    b.Append("<path d=\""+Esc(data)+"\" stroke=\""+Color(shape.Stroke)+"\" fill=\""+Color(shape.Fill)+"\" stroke-width=\""+N(shape.StrokeThickness)+"\" stroke-linecap=\"round\" stroke-linejoin=\"round\""+transform+"/>\n");
   }else if(shape is Ellipse){
    b.AppendFormat(C,"<ellipse cx=\"{0}\" cy=\"{1}\" rx=\"{2}\" ry=\"{3}\" fill=\"{4}\" stroke=\"{5}\" stroke-width=\"{6}\"{7}/>\n",N(shape.Margin.Left+shape.Width/2),N(shape.Margin.Top+shape.Height/2),N(shape.Width/2),N(shape.Height/2),Color(shape.Fill),Color(shape.Stroke),N(shape.StrokeThickness),transform);
   }else throw new InvalidOperationException("Unsupported shape");
  }b.Append("</g>\n");
 }
 [STAThread]static int Main(){
  try{
   RenderOptions.ProcessRenderMode=System.Windows.Interop.RenderMode.SoftwareOnly;
   string root=AppDomain.CurrentDomain.BaseDirectory,source=System.IO.Path.Combine(root,"src","VectorIcons989.cs"),outdir=System.IO.Path.Combine(root,"assets");
   string[] names=Regex.Matches(File.ReadAllText(source),"case \"([^\"]+)\":").Cast<Match>().Select(m=>m.Groups[1].Value).ToArray();
   if(names.Length!=13)throw new Exception("Unexpected icon inventory");
   int cols=4,cellW=240,cellH=166,margin=24,top=98,width=1008,height=804;
   var canvas=new Canvas{Width=width,Height=height,Background=B("#161A20")};
   Place(canvas,Text("Smooth Motion · vector icon inventory",24,"#F0F3F7"),margin,22);
   Place(canvas,Text("Actual UI geometry | 72 px detail + 18 / 24 / 32 px sizes | R989",14,"#9FABBB"),margin,60);
   var svg=new StringBuilder();svg.Append("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1008\" height=\"804\" viewBox=\"0 0 1008 804\">\n<title>Current R989 actual vector icons</title>\n");
   svg.Append("<rect width=\"1008\" height=\"804\" fill=\"#161A20\"/><g font-family=\"Segoe UI,sans-serif\"><text x=\"24\" y=\"48\" font-size=\"24\" fill=\"#F0F3F7\">Smooth Motion · vector icon inventory</text><text x=\"24\" y=\"76\" font-size=\"14\" fill=\"#9FABBB\">Actual UI geometry | 72 px detail + 18 / 24 / 32 px sizes | R989</text></g>\n");
   for(int i=0;i<names.Length;i++){
    double x=margin+(i%cols)*cellW,y=top+(i/cols)*cellH;
    Place(canvas,new Border{Width=cellW-12,Height=cellH-12,CornerRadius=new CornerRadius(8),Background=B("#20262D"),BorderBrush=B("#34404D"),BorderThickness=new Thickness(1)},x,y);
    Place(canvas,Text(names[i],15,"#DBE2EA"),x+15,y+10);
    Place(canvas,VectorIcons989.Create(names[i],72),x+14,y+49);
    double[] sizes={18,24,32};double[] offsets={110,143,183};
    svg.AppendFormat(C,"<rect x=\"{0}\" y=\"{1}\" width=\"228\" height=\"154\" rx=\"8\" fill=\"#20262D\" stroke=\"#34404D\"/><text x=\"{2}\" y=\"{3}\" fill=\"#DBE2EA\" font-family=\"Segoe UI,sans-serif\" font-size=\"15\">{4}</text>\n",x,y,x+15,y+28,Esc(names[i]));
    SvgIcon(svg,names[i],x+14,y+49,72);
    for(int k=0;k<sizes.Length;k++){
     Place(canvas,VectorIcons989.Create(names[i],sizes[k]),x+offsets[k],y+79-sizes[k]/2);
     Place(canvas,Text(N(sizes[k]),11,"#8C9AAC"),x+offsets[k]+1,y+110);
     SvgIcon(svg,names[i],x+offsets[k],y+79-sizes[k]/2,sizes[k]);
     svg.AppendFormat(C,"<text x=\"{0}\" y=\"{1}\" fill=\"#8C9AAC\" font-family=\"Segoe UI,sans-serif\" font-size=\"11\">{2}</text>\n",x+offsets[k]+1,y+122,N(sizes[k]));
    }
   }
   Place(canvas,Text("PNG: WPF SoftwareOnly. SVG: exported from those same Path / Ellipse objects. No UI or engine changes.",12,"#8795A7"),margin,height-27);
   svg.Append("<text x=\"24\" y=\"789\" fill=\"#8795A7\" font-family=\"Segoe UI,sans-serif\" font-size=\"12\">PNG: WPF SoftwareOnly. SVG: exported from the same Path / Ellipse objects. No UI or engine changes.</text>\n</svg>\n");
   canvas.Measure(new Size(width,height));canvas.Arrange(new Rect(0,0,width,height));canvas.UpdateLayout();
   foreach(int scale in new[]{1,2}){
    var bitmap=new RenderTargetBitmap(width*scale,height*scale,96*scale,96*scale,PixelFormats.Pbgra32);bitmap.Render(canvas);
    var png=new PngBitmapEncoder();png.Frames.Add(BitmapFrame.Create(bitmap));
    using(var file=File.Create(System.IO.Path.Combine(outdir,"vector-icons989"+(scale==1?"":"@2x")+".png")))png.Save(file);
   }
   File.WriteAllText(System.IO.Path.Combine(outdir,"vector-icons989.svg"),svg.ToString(),new UTF8Encoding(false));
   File.WriteAllText(System.IO.Path.Combine(outdir,"vector-icons989-manifest.json"),"{\"source\":\"src/VectorIcons989.cs\",\"source_sha256\":\""+Hash(source)+"\",\"icon_count\":"+names.Length+",\"icons\":[\""+String.Join("\",\"",names)+"\"],\"width\":"+width+",\"height\":"+height+",\"rendering\":\"WPF SoftwareOnly, no window, no GPU inference\",\"svg_geometry\":\"Exported from live VectorIcons989 Path/Ellipse instances, no redrawing\",\"desktop_source_modified\":false}",new UTF8Encoding(false));
   Console.WriteLine("PASS 13 actual vector icons; PNG 1008x804 / 2016x1608 + SVG; source "+Hash(source));return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
 }
}
