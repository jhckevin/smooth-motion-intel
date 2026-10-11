using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
namespace SmoothMotionDesktop {
 internal static class VectorIcons989 {
  static readonly Brush Ink=Frozen("#E7E9EC"),Soft=Frozen("#8EA3B9"),Accent=Frozen("#8BDEDC");
  static Brush Frozen(string value){var b=(SolidColorBrush)new BrushConverter().ConvertFromString(value);b.Freeze();return b;}
  static Path Stroke(string data,Brush color,double width){var g=Geometry.Parse(data);g.Freeze();return new Path{Data=g,Stroke=color,StrokeThickness=width,StrokeStartLineCap=PenLineCap.Round,StrokeEndLineCap=PenLineCap.Round,StrokeLineJoin=PenLineJoin.Round,Stretch=Stretch.None};}
  static void Add(Canvas canvas,string data,Brush color,double width){canvas.Children.Add(Stroke(data,color,width));}
  public static FrameworkElement Create(string name,double size=18){
   var c=new Canvas{Width=24,Height=24,ClipToBounds=false};
   switch(name){
    case "logo":
     Add(c,"M5,7 L5,4 Q5,2 7,2 L19,2 Q21,2 21,4 L21,16 Q21,18 19,18 L17,18",Soft,1.4);
     Add(c,"M2,10 L2,8 Q2,6 4,6 L16,6 Q18,6 18,8 L18,20 Q18,22 16,22 L4,22 Q2,22 2,20 L2,17",Ink,1.45);
     Add(c,"M1,14 L6,14 M14,14 L21,14 M9.8,9.6 L14.2,14 L9.8,18.4 L5.4,14 Z",Accent,1.6);
     Add(c,"M9.8,11.8 L9.8,16.2 M7.6,14 L12,14",Ink,1.25);
     c.Children.Add(new Ellipse{Width=2.5,Height=2.5,Fill=Accent,Margin=new Thickness(20,12.75,0,0)});
     break;
    case "minimize":Add(c,"M5,16 L19,16",Ink,1.45);break;
    case "maximize":Add(c,"M5.5,5.5 L18.5,5.5 L18.5,18.5 L5.5,18.5 Z",Ink,1.45);break;
    case "restore":Add(c,"M8,5 L19,5 L19,16 M5,8 L16,8 L16,19 L5,19 Z",Ink,1.45);break;
    case "close":Add(c,"M6,6 L18,18 M18,6 L6,18",Ink,1.65);break;
    case "add":Add(c,"M12,5 L12,19 M5,12 L19,12",Ink,1.7);break;
    case "edit":Add(c,"M4,16 L4,20 L8,20 L19,9 Q20,8 19,7 L17,5 Q16,4 15,5 Z M13,7 L17,11 M4,16 L8,20",Ink,1.55);break;
    case "delete":Add(c,"M5,7 L19,7 M9,4 L15,4 M7,7 L8,20 L16,20 L17,7 M10,10 L10.5,17 M14,10 L13.5,17",Ink,1.55);break;
    case "manual":Add(c,"M12,6 Q8,3 3,5 L3,19 Q8,17 12,20 Q16,17 21,19 L21,5 Q16,3 12,6 L12,20 M6,8 L9,9 M6,12 L9,13 M15,9 L18,8 M15,13 L18,12",Ink,1.4);break;
    case "settings":Add(c,"M9,3 L15,3 L15.7,5.5 L18,6.8 L20.5,6.2 L23,10.8 L21,12.6 L21,15 L23,16.8 L20.5,21 L17.9,20.5 L15.7,21.8 L15,24 L9,24 L8.3,21.8 L6.1,20.5 L3.5,21 L1,16.8 L3,15 L3,12.6 L1,10.8 L3.5,6.2 L6,6.8 L8.3,5.5 Z",Ink,1.3);
     // Keep the detailed gear within a 24 px optical box.
     c.Children[c.Children.Count-1].RenderTransform=new MatrixTransform(.83,0,0,.83,2,.1);
     c.Children.Add(new Ellipse{Width=6.8,Height=6.8,Stroke=Ink,StrokeThickness=1.55,Margin=new Thickness(8.6,8.6,0,0)});break;
    case "back":Add(c,"M14.5,5 L7.5,12 L14.5,19 M8,12 L21,12",Ink,1.65);break;
    case "refresh":Add(c,"M19,9 A7.2,7.2 0 1 0 18.8,16 M19,4 L19,9 L14,9",Ink,1.65);break;
    case "window":Add(c,"M4,4 L20,4 L20,20 L4,20 Z M4,9 L20,9 M7,6.5 L7.1,6.5 M10,6.5 L10.1,6.5",Ink,1.5);break;
    default:Add(c,"M4,4 L20,4 L20,20 L4,20 Z",Ink,1.5);break;
   }
   var v=new Viewbox{Width=size,Height=size,Stretch=Stretch.Uniform,Child=c,HorizontalAlignment=HorizontalAlignment.Center,VerticalAlignment=VerticalAlignment.Center,IsHitTestVisible=false};
   return v;
  }
 }
}
