using System;
using System.IO;
using System.Text;
using System.Windows;
using System.Security.Cryptography;
using System.Web.Script.Serialization;

namespace SmoothMotionDesktop {
 public static class ModelNotice996 {
  const string Revision="converted-model-notice-996-1";
  static string Hash(byte[] bytes){using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(bytes)).Replace("-","").ToLowerInvariant();}
  static void PlainPath(string path){
   for(string p=Path.GetFullPath(path);!String.IsNullOrEmpty(p);p=Path.GetDirectoryName(p)){
    if((File.Exists(p)||Directory.Exists(p))&&(File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0)throw new IOException("Model notice receipt cannot use a linked path.");
   }
  }
  public static bool Confirm(string appRoot){return Confirm(appRoot,(text,buttons)=>MessageBox.Show(text,"Smooth Motion — 模型使用前声明 / Model notice",buttons,MessageBoxImage.Information,buttons==MessageBoxButton.OK?MessageBoxResult.OK:MessageBoxResult.Cancel));}
  internal static bool Confirm(string appRoot,Func<string,MessageBoxButton,MessageBoxResult> prompt){
   if(!File.Exists(Path.Combine(appRoot,"model","graph922.bin")))return true;
   string notice=Path.Combine(appRoot,"MODEL-NOTICE.md");
   if(!File.Exists(notice)){prompt("缺少 MODEL-NOTICE.md，无法展示模型来源和使用声明。\nMissing MODEL-NOTICE.md. Restore the complete application bundle.",MessageBoxButton.OK);return false;}
   string digest=Hash(File.ReadAllBytes(notice)),dir=Path.Combine(appRoot,"UserData"),receipt=Path.Combine(dir,"model-notice996.json");
   PlainPath(receipt);
   if(File.Exists(receipt)&&new FileInfo(receipt).Length<8192){
    try {var row=new JavaScriptSerializer().DeserializeObject(File.ReadAllText(receipt)) as System.Collections.Generic.Dictionary<string,object>;
     if(row!=null&&row.ContainsKey("revision")&&row.ContainsKey("notice_sha256")&&Convert.ToString(row["revision"])==Revision&&Convert.ToString(row["notice_sha256"])==digest)return true;
    }catch{}
   }
   string message="本包附带已转换的 Smooth Motion 权重、计算图和 Intel Vulkan/XMX SPIR-V；不加载原装 NVIDIA DLL，也不自动提取模型。\n\n维护者免费提供，非官方产品。第三方模型不适用项目原创代码的 Apache-2.0 许可。非盈利、转换和免责声明不构成厂商授权；本项目不宣称已取得额外转换或再分发许可。\n\n软件与模型按原样提供，可能出现画面瑕疵、延迟或设备错误。请先阅读程序目录中的 MODEL-NOTICE.md（中英文），自行确认适用条款与风险。\n\nThis bundle contains a converted model, not the original NVIDIA DLL. It is free and unofficial. Apache-2.0 does not license third-party model assets. Non-profit distribution and this notice grant no missing rights. Read MODEL-NOTICE.md before proceeding.\n\n“确定 / OK”仅表示已阅读并继续；“取消 / Cancel”退出。";
   if(prompt(message,MessageBoxButton.OKCancel)!=MessageBoxResult.OK)return false;
   Directory.CreateDirectory(dir);PlainPath(receipt);
   string json=new JavaScriptSerializer().Serialize(new{revision=Revision,notice_sha256=digest,acknowledged_utc=DateTime.UtcNow.ToString("o"),meaning="read acknowledgement only; not a rights grant"});
   File.WriteAllText(receipt,json,new UTF8Encoding(false));return true;
  }
 }
}
