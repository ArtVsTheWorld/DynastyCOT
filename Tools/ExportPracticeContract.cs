using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Web.Script.Serialization;
class ExportPracticeContract {
 static int Main(string[] args) {
  Assembly a=Assembly.LoadFrom(Path.GetFullPath(args[0]));Type t=a.GetType("SituationalPractice.NativeRuntime",true);
  long image=0x140000000L,data=0x200000000L,delta=0x10000000L;
  long[] sites=(long[])t.GetField("Sites").GetValue(null);byte[][] originals=(byte[][])t.GetField("Originals").GetValue(null);
  var blocks=new List<object>();
  foreach(int index in new[]{0,13,14}) {
   string name=index==0?"BuildGate":index==13?"BuildModeDispatch":"BuildModeText";
   MethodInfo method=t.GetMethod(name);var variants=new List<string>();
   for(int variant=0;variant<4;variant++){
    long b=image+((variant&1)!=0?delta:0),d=data+((variant&2)!=0?delta:0);
    object[] input=index==0?new object[]{b,d,0}:new object[]{b,d};
    variants.Add(BitConverter.ToString((byte[])method.Invoke(null,input)).Replace("-",""));
   }
   blocks.Add(new Dictionary<string,object>{{"index",index},{"name",name},{"rva",sites[index]},
    {"original",BitConverter.ToString(originals[index]).Replace("-","")},{"variants",variants}});
  }
  File.WriteAllText(args[1],new JavaScriptSerializer().Serialize(new Dictionary<string,object>{
   {"image",image},{"data",data},{"delta",delta},{"blocks",blocks}}));
  return 0;
 }
}
