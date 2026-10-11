using System;
using System.IO;
using System.Windows;
using SmoothMotionDesktop;
class ModelNotice996Tests {
 static int checks;
 static void Check(bool value,string reason){checks++;if(!value)throw new Exception(reason);}
 static int Main(string[] args){
  string root=Path.Combine(Path.GetFullPath(args[0]),"case-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
  int prompts=0;Func<string,MessageBoxButton,MessageBoxResult> deny=(text,buttons)=>{prompts++;return MessageBoxResult.Cancel;};
  Check(ModelNotice996.Confirm(root,deny)&&prompts==0,"source-only install should not prompt");
  Directory.CreateDirectory(Path.Combine(root,"model"));File.WriteAllText(Path.Combine(root,"model","graph922.bin"),"synthetic marker, not a model");
  Check(!ModelNotice996.Confirm(root,deny)&&prompts==1,"missing notice must block before model use");
  File.WriteAllText(Path.Combine(root,"MODEL-NOTICE.md"),"notice A");
  Check(!ModelNotice996.Confirm(root,deny),"decline must block");
  string receipt=Path.Combine(root,"UserData","model-notice996.json");
  Check(!File.Exists(receipt),"decline must not record acknowledgement");
  Func<string,MessageBoxButton,MessageBoxResult> allow=(text,buttons)=>{prompts++;Check(text.Contains("不构成厂商授权")&&text.Contains("no missing rights"),"dialog must not imply permission");return MessageBoxResult.OK;};
  Check(ModelNotice996.Confirm(root,allow)&&File.Exists(receipt),"acknowledgement must persist");
  int prior=prompts;Check(ModelNotice996.Confirm(root,deny)&&prompts==prior,"same notice should not reprompt");
  string priorReceipt=File.ReadAllText(receipt);
  File.WriteAllText(Path.Combine(root,"MODEL-NOTICE.md"),"notice B");
  Check(!ModelNotice996.Confirm(root,deny),"changed notice must require new acknowledgement");
  Check(File.ReadAllText(receipt)==priorReceipt,"decline changed notice must preserve old record");
  Check(ModelNotice996.Confirm(root,allow),"changed notice accepted");
  File.WriteAllText(receipt,"malformed");
  Check(!ModelNotice996.Confirm(root,deny),"malformed receipt must not bypass");
  Console.WriteLine("{\"pass\":true,\"checks\":"+checks+",\"gpu_used\":false,\"real_dialog_shown\":false}");return 0;
 }
}
