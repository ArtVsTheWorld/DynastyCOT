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
        // October 29 layout, from the inspected unmodified native memory capture.
        // Contiguous unwind fragments are joined; leaf routines have exact bounds.
        // Call Sheet / Dynasty Hooks redirect two complete five-byte CALLs at
        // 0x7b82504 (CommitPlay) and 0x3601e2f (auto-playcall query). Exclude
        // only those instructions; retain every surrounding byte and all of
        // our hook entry checks. Gameplay hooks are unchanged.
        internal static readonly Region[] Regions={
            new Region(0x7b836b0, 40, "83d0b35ff29d488d4dba4324f735f2abbdf5f25d711aeaadd062d122fff833ed"),
            new Region(0x35c24b0, 514, "917b60c8b6b3f6bf22d8ec15f3ead404f4523e9a6290dbb2a7f7bf97de3a30f0"),
            new Region(0x5fe1bd0, 232, "48df33cbf4b296ba01a977a4537df3200d9a4611e06d3bbbb2f464e7b6ad5b72"),
            new Region(0x2ad9200, 5, "22bdee30f3c1dd6de8568705cde8f9633bc6c8172fa06df18a8a27833dccb072"),
            new Region(0x34875e0, 86, "40fcfa2d50110b60c6d554ec26abbf9d25820ed360b81c54f492ae7c0928adfd"),
            new Region(0x35e3330, 445, "274d26d97b136ef7f555c8540978c0863ca9d3d879513d2fcd62436a0ccc3087"),
            new Region(0x35e4850, 368, "fa3ee8d6e4e77029f23e1b5560852ae6a4daf9987a7b761f7a2602ab7ad1c909"),
            new Region(0x35ff940, 3356, "0d81a9a969cbcce0a673fe1cc3eab4cf7334acfeb171d9bc22e892e58e84db0e"),
            new Region(0x3601240, 1561, "f4b9e86fc67ecf26702a8098a54af15f5f680be842c6c7a297967bd297fe121e"),
            new Region(0x3601860, 1487, "bc8f225aaceb454dbe417f10f05f0e413a9c36eb90793812af6c062896f86daa"),
            new Region(0x3601e34, 486, "4127a282d9d904bcdac574a39a02e90f7a2055c0aef219a1844b20cccb8cba03"),
            new Region(0x36a1450, 1211, "1fc0e1558f537fd762ecbf32e95d188188c9f78e3f6c33227a80f644164f74a7"),
            new Region(0x377e340, 136, "3f56c3b2bcd0d5bd6cba8f602c80356094b300ff1c376762a68017369f4f9c0f"),
            new Region(0x3f0a300, 8, "6dd01991fadf6f57785ac911ed232130421e67baf1c8524a86b3f716e6c8beb8"),
            new Region(0x4ff6fc0, 1063, "37612e52c7247126114b553fb299d33d7993c14f9bd988d9ca26c021dcd8a7d9"),
            new Region(0x4ff7400, 262, "9652ed5cfe52a4ecf9cc3c83baef6f3afd18a8c9f4289723abcd495c9d262512"),
            new Region(0x4ff76e0, 783, "6c6acfe612bce7bbb7fdd06c9733b297053e79ad38d959613a36e020baf73c56"),
            new Region(0x4ff8c10, 89, "5512402dec41f3e9dac2e2d469d82636aad556906937d29d21ad118f15d989f2"),
            new Region(0x5d12420, 271, "938744e5509d3b476f1ccfd80740fcce961c2e3940ddac1230dd74c949478774"),
            new Region(0x5d4d900, 516, "ad65b803b3ee886cb75549039de0096d9a26b10f6dab6f365c7b723ac7221ae6"),
            new Region(0x5d5d000, 62, "a197bbdfaaa5600b15535647e1f21decc7eba8c988b1c3b0ffa4e65f956eb554"),
            new Region(0x5d5d040, 79, "0ef5dbb7f79e6ac8a51e38282734b68e6932a99ab978830736f51ad5bba4bc04"),
            new Region(0x5d5d090, 1156, "23b7e404b2b9a4377732df9f04b7c0a018a65d8f2a403472b4628d725630ae70"),
            new Region(0x5d5e570, 1053, "fac2d4ec406b868650d8233c4a2ea93fc9434c7706bda95eed80d86710d26655"),
            new Region(0x5d70550, 2019, "7cef7aa1fea5362637c03a1eacea529207f4e401a64c04b320325adc945bc47c"),
            new Region(0x5d74320, 7044, "53f660da6e71d2ee3a81df2282ca8d4a80dfb618ed05a34fa4f34a81920af851"),
            new Region(0x5f0d5c0, 690, "ac873634d791e9aeae6ea34021e730f7a82192992842e2b64051edf70b1e14a1"),
            new Region(0x7b82380, 388, "84849acc2335d10ff0eef36eddf3281b2051fa5d32416001c75e4720bc1c11c7"),
            new Region(0x7b82509, 44, "4303b38a74a0bea03a4ff9ddb9aee117771868dfdd67d70cd59c9fb46fecfddb"),
            new Region(0x7b9d930, 561, "9a64e3c3e93ae5691323e4337dc697af5caabd090aa92fbb5bd5e4837af2db83"),
            new Region(0x7baf6d0, 168, "cf2ccaf8f011f8a9e4771c68ff28faa8814be5b0c3a77bae3fc979d80fec298d"),
            new Region(0x7baf940, 103, "b0c02742cb9f74fe00ea029aaea8afe15f336e260dd0c50209ecf44ce8c009f3"),
            new Region(0xae3a190, 16, "231f5cecc61699ca90f5e9acdbc4c47552f66daeb7ef55327cb7574cda9f29e5"),
            new Region(0xbab7020, 16, "0f023d55ec1805f2c2fbbc1debe1075cfc153ebb81b7814f6f5b9bf7d613146b"),
            new Region(0xbab7030, 16, "50ec86dca0a9e4d7818f30e38545007351cfd8c06883650b09d93f2789a41c3c"),
            new Region(0xbab7040, 16, "27d6d18d488331575c4684105e46a16e69d9aa9c7b503590130d7081ec229bae"),
            new Region(0xbab7050, 16, "c8a6e2851fe308ff9d0e7ce942774e14ca5d4f0655acf8167eae615207bae265"),
            new Region(0x5d5d9f0, 63, "32077d3f7b44fdc7a18800dcbba2bffa94a37e11bb1f581f5badecd62a8a0a90"),
            new Region(0x7bce8f0, 72, "96b0fe3e0c1ab4f1090ffbd8ac49c1ceb9e71a529035eaefe848f7663003d96a"),
            new Region(0x7c1b8c0, 8, "0114f7b6fcd000b5da40a5c37b9d506e68f1cdf1c2833b4d71ee5dfc6a46b008"),
            new Region(0x25a3e60, 168, "3b8537229ab2aeee3325e5057944c424b3864ef14094bae8da1e0bd2bd142645"),
            new Region(0x5d4e130, 596, "af69acf3614208e7d1d0bc794c6b9891518848a4d3c8a6286f886ad9acf8acf5"),
        };
        internal static void Verify(Func<long,int,byte[]> read,long image,Func<long,bool> extensionOwner=null)
        {
            using(var sha=SHA256.Create())foreach(var region in Regions) {
                byte[] bytes=read(image+region.Rva,region.Length);
                if(bytes==null || bytes.Length!=region.Length)
                    throw new InvalidOperationException("Cannot read required native code at RVA "+region.Rva.ToString("x")+". No hooks installed.");
                if(region.Rva==0x3601240)bytes=ExtensionsPregame.Normalize(read,image,bytes,extensionOwner);
                if(region.Rva==0x5d12420)bytes=DynastyControllerCompatibility.Normalize(read,image,bytes);
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
   long relay=image+0x360128b+5+BitConverter.ToInt32(actual,1);var code=read(relay,12);
   if(code==null||code.Length!=12||code[0]!=0x48||code[1]!=0xb8||code[10]!=0xff||code[11]!=0xe0)throw new InvalidOperationException("Unsupported Extensions pregame trampoline; no hooks installed.");
   long destination=BitConverter.ToInt64(code,2);var expected=Hex("4157415641554154555756534881ecc80000004889ce488d0d97470200e8fe67ffff8b0518da010083f8ff0f85570100008b05e5b2030085c00f8549010000448b1d22ea03008b1dbcb603004585db741f8b057dda010083f8ff74148b158ada");
   if(!owner(destination)||!Equal(read(destination,expected.Length),expected))throw new InvalidOperationException("Unverified Extensions pregame destination; no hooks installed.");
   var copy=(byte[])region.Clone();Array.Copy(Original(),0,copy,offset,5);return copy;
  }
  static byte[] Hex(string text){var bytes=new byte[text.Length/2];for(int i=0;i<bytes.Length;++i)bytes[i]=Convert.ToByte(text.Substring(i*2,2),16);return bytes;}
 }
}
