using File = Ace.DynastyCOT.StorageFile;
using System;
using System.Security.Cryptography;
using System.Diagnostics;
using System.IO;

namespace SituationalPractice
{
    // Verify the decrypted code we actually use, independent of the EXE's
    // container/signature/other sections. Hook entry ownership is checked separately.
    internal static class NativeCompatibility
    {
        internal sealed class Region
        {
            internal readonly long Rva;
            internal readonly int Length;
            internal readonly string Hash;
            internal Region(long rva,int length,string hash){Rva=rva;Length=length;Hash=hash;}
        }
        // November 19 layout, from the inspected unmodified native memory capture.
        // Contiguous unwind fragments are joined; leaf routines have exact bounds.
        // Call Sheet / Dynasty Hooks redirect two complete five-byte CALLs at
        // 0x7b82594 (CommitPlay) and 0x3601ebf (auto-playcall query). Exclude
        // only those instructions; retain every surrounding byte and all of
        // our hook entry checks. Gameplay hooks are unchanged.
        internal static readonly Region[] Regions={
            new Region(0x7b83740, 40, "83d0b35ff29d488d4dba4324f735f2abbdf5f25d711aeaadd062d122fff833ed"),
            new Region(0x35c2540, 514, "e6fd8dc69e969f0a46f9a582d286dd6690535b3b813e0698647733f9c16983d8"),
            new Region(0x5fe1c60, 232, "7271d1b51b7e82027ee9905c6e49c4bb78ef13eaf29333121270c2e11844005a"),
            new Region(0x2ad9200, 5, "22bdee30f3c1dd6de8568705cde8f9633bc6c8172fa06df18a8a27833dccb072"),
            new Region(0x3487670, 86, "40fcfa2d50110b60c6d554ec26abbf9d25820ed360b81c54f492ae7c0928adfd"),
            new Region(0x35e33c0, 445, "0396bde8258fd2ec5f472088a6de181c37229f82e92a2ed91424a80cf8763ba3"),
            new Region(0x35e48e0, 368, "9511b69eb70bcfb5dc3d9cae2528bd5b86e3e80242e11e01cf9285a4995b4fb2"),
            new Region(0x35ff9d0, 3356, "d75c44781e6c75231581af2cba186f3f2cb93191a155d669db25055a67d997a3"),
            new Region(0x36012d0, 1561, "c7ff6aa0bf9cc1e5473bab8b8ec50f530042982820ab4c0633d5a325ebeab02b"),
            new Region(0x36018f0, 1487, "02d393ce65fc2fcacad0adef3229cd67925a3cc6c3faf4d4340bb99291f1282f"),
            new Region(0x3601ec4, 486, "b40dfb016491cdd2a898a0587f9598d1176535180542f96bb456da32218d12aa"),
            new Region(0x36a14e0, 1211, "1d864976752789fa7afc8f52f208ccaf53e5fea1f1d86b3a44faccb20af0d208"),
            new Region(0x377e3d0, 136, "60f383104c6080705dd93efb156c32ba9703e023b100e7d570977ce202c1d1e2"),
            new Region(0x3f0a390, 8, "514867708172d5de356d4fc10e810c2b5fd7c13eda8535def41949abb4a9a30a"),
            new Region(0x4ff7050, 1063, "46fd61b8bab989df450afd60156db7ac84bfdeaf88f0939447864a56f4134712"),
            new Region(0x4ff7490, 262, "fadc8c2e119da0a50352a0458b906ba0bf06b33a2de66e8d6c3c23553bdbf871"),
            new Region(0x4ff7770, 783, "bc115b2929d61e1944e9f2d36f9f17b3227e02ea0ebdac4a53996076aaf6ec94"),
            new Region(0x4ff8ca0, 89, "a8aa006db06380209437b2de7527c6511047128ed39afa4fa12b15e7f806a0fb"),
            new Region(0x5d124b0, 271, "260e8b3c8d8ed4cb19ae57a407fc375e12d94aef6614c49205a071dae4cda419"),
            new Region(0x5d4d990, 516, "541d2c09a208cadc3298c967be0e858c26818fd106914b50c3c1c7bf7f59913a"),
            new Region(0x5d5d090, 62, "ae8a2874c93cbd6bd4a35de91dcbc9da9018b75885492cd73baca841b05b4525"),
            new Region(0x5d5d0d0, 79, "b25c226300e5935e50f1bff7a53349be213c3bc7ecdad9f2ac14851dd0727cfd"),
            new Region(0x5d5d120, 1156, "645ef8ba8b1962ea12aba789730179b3a62e3a2eae2dd124b1bc5a4fcc2818f5"),
            new Region(0x5d5e600, 1053, "c73cc7000f5d1333cdfe1f87f358ea7ec428a314a03ad4177076c108e001720e"),
            new Region(0x5d705e0, 2019, "4f27a4942f6eb7ee37a7ec2b7d8ab2c2e7cbd3a8f9bc9f6b2f2fdac04ceedd99"),
            new Region(0x5d743b0, 7044, "99898522833edab44b4370fb6780a90e640c8dc8cb67b79d70adecb07973a19f"),
            new Region(0x5f0d650, 690, "60017636e9eb50c55b2cbc6ec8b72ed978180204eccdc56cdd15f29e820e5cc5"),
            new Region(0x7b82410, 388, "207b953483d0ba2d8674f9f969b7956002ba6fd38ef20f3c5a9e7f5dfeb9c1a7"),
            new Region(0x7b82599, 44, "4303b38a74a0bea03a4ff9ddb9aee117771868dfdd67d70cd59c9fb46fecfddb"),
            new Region(0x7b9d9c0, 561, "1a5dbcba7a6cfc5e900786ea0a12e4fe4070b00224a5b06551b37bf09ed250d0"),
            new Region(0x7baf760, 168, "f8fb51b9bf915cc19b4025ede5d84957a56860765d8cba83888dee2070187dfe"),
            new Region(0x7baf9d0, 103, "bb71ac726bbe8f76606b6eb4a83441248e05c990dc446d81bdcdabed7c1b3310"),
            new Region(0xae3a1b0, 16, "231f5cecc61699ca90f5e9acdbc4c47552f66daeb7ef55327cb7574cda9f29e5"),
            new Region(0xbab6fc0, 16, "0f023d55ec1805f2c2fbbc1debe1075cfc153ebb81b7814f6f5b9bf7d613146b"),
            new Region(0xbab6fd0, 16, "50ec86dca0a9e4d7818f30e38545007351cfd8c06883650b09d93f2789a41c3c"),
            new Region(0xbab6fe0, 16, "27d6d18d488331575c4684105e46a16e69d9aa9c7b503590130d7081ec229bae"),
            new Region(0xbab6ff0, 16, "c8a6e2851fe308ff9d0e7ce942774e14ca5d4f0655acf8167eae615207bae265"),
            new Region(0x5d5da80, 63, "0f3a5262b29f65e49dc2f63509d46030a33e08dbd9d730e6cf590927cdf8851e"),
            new Region(0x7bce980, 72, "47a76d24b63bf1c073b399cf6b7ab668ea018d3d78af592c90563bebdf137cf4"),
            new Region(0x7c1b950, 8, "1ee1e834dd285bc898bd18a11d0cb3a15a32fc6a428ac30311bb9cffb382515d"),
            new Region(0x25a3e60, 168, "3f023cf3603a4fe524f9c29f83d600946bd86791aa0cf882b12e3824c9c1baa7"),
            new Region(0x5d4e1c0, 596, "bdafb3670a5522e5bc124110c514f58de7aa012362ead22c0e71e8fb7382e2ba"),
        };
        internal static void Verify(Func<long,int,byte[]> read,long image,Func<long,bool> extensionOwner=null)
        {
            using(var sha=SHA256.Create())foreach(var region in Regions) {
                byte[] bytes=read(image+region.Rva,region.Length);
                if(bytes==null || bytes.Length!=region.Length)
                    throw new InvalidOperationException("Cannot read required native code at RVA "+region.Rva.ToString("x")+". No hooks installed.");
                if(region.Rva==0x36012d0)bytes=ExtensionsPregame.Normalize(read,image,bytes,extensionOwner);
                if(region.Rva==0x5d124b0)bytes=DynastyControllerCompatibility.Normalize(read,image,bytes);
                string actual=BitConverter.ToString(sha.ComputeHash(bytes)).Replace("-","").ToLowerInvariant();
                if(actual!=region.Hash)
                    throw new InvalidOperationException("Required native code differs at RVA "+region.Rva.ToString("x")+" (expected "+region.Hash+", actual "+actual+"). Waiting for compatible decrypted code; no hooks installed.");
            }
        }
    }
}



namespace SituationalPractice {
 internal static class ExtensionsPregame {
  internal const string SupportedSha256="170633be325803b94b0389c39d9ace4ce20c676993b46e533fa14d6fafe690a0";
  static byte[] Original(){return new byte[]{0xe8,0xf0,0xba,0x0,0x0};}
  static bool Equal(byte[] a,byte[] b){if(a==null||b==null||a.Length!=b.Length)return false;for(int i=0;i<a.Length;++i)if(a[i]!=b[i])return false;return true;}
  internal static bool Owner(Process game,long destination){
   foreach(ProcessModule module in game.Modules){if(destination!=module.BaseAddress.ToInt64()+0xe620||module.ModuleMemorySize!=344064)continue;
    using(var file=File.OpenRead(module.FileName))using(var sha=SHA256.Create())if(BitConverter.ToString(sha.ComputeHash(file)).Replace("-","").ToLowerInvariant()==SupportedSha256)return true;
   }return false;
  }
  internal static byte[] Normalize(Func<long,int,byte[]> read,long image,byte[] region,Func<long,bool> owner){
   const int offset=0x4b;if(region==null||region.Length!=1561)return region;
   var actual=new byte[5];Array.Copy(region,offset,actual,0,5);if(Equal(actual,Original()))return region;
   if(actual[0]!=0xe8||owner==null)throw new InvalidOperationException("Unknown pregame wrapper at RVA 360128b; no hooks installed.");
   long relay=image+0x360131b+5+BitConverter.ToInt32(actual,1);var code=read(relay,12);
   if(code==null||code.Length!=12||code[0]!=0x48||code[1]!=0xb8||code[10]!=0xff||code[11]!=0xe0)throw new InvalidOperationException("Unsupported Extensions pregame trampoline; no hooks installed.");
   long destination=BitConverter.ToInt64(code,2);var expected=Hex("4157415641554154555756534881ecc80000004889ce488d0d97470200e8fe67ffff8b0518da010083f8ff0f85570100008b05e5b2030085c00f8549010000448b1d22ea03008b1dbcb603004585db741f8b057dda010083f8ff74148b158ada");
   if(!owner(destination)||!Equal(read(destination,expected.Length),expected))throw new InvalidOperationException("Unverified Extensions pregame destination; no hooks installed.");
   var copy=(byte[])region.Clone();Array.Copy(Original(),0,copy,offset,5);return copy;
  }
  static byte[] Hex(string text){var bytes=new byte[text.Length/2];for(int i=0;i<bytes.Length;++i)bytes[i]=Convert.ToByte(text.Substring(i*2,2),16);return bytes;}
 }
}
