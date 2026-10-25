/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */


#ifndef GUCEF_CORE_TESTAPP_TESTRSAVECTORS_H
#define GUCEF_CORE_TESTAPP_TESTRSAVECTORS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEST DATA                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  RSA public keys and PKCS#1 v1.5 signatures. Keys A and B are 2048 bit, the signatures of key A over g_rsaMessage
 *  with SHA-256, SHA-384 and SHA-512 are the output of an independent implementation (OpenSSL) and equal the
 *  textbook EMSA-PKCS1-v1_5 encoding signed with the private key. The "Crafted" signatures are private key operations
 *  over encoded messages that are not valid EMSA-PKCS1-v1_5 encodings of g_rsaMessage. The Google style token is an
 *  RS256 token with the claims a Google ID token has, signed with key A.
 *  The RFC 7515 values are those of appendix A.2.
 *  Hexadecimal text is big endian bytes.
 */

/** RFC 7515 appendix A.2: the JWS signing input, the base64url of n (e is AQAB) and of the signature */
static const char* const g_rfc7515A2SigningInput = "eyJhbGciOiJSUzI1NiJ9.eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFtcGxlLmNvbS9pc19yb290Ijp0cnVlfQ";

static const char* const g_rsaKeyAModulusHex =
    "a4faeab9746bd0609ea8be106b63188a660b4860039ba0b2f489175c206bbe7c5fbeb8f0f299847ca2572a6e53de6014"
    "41389823788915e57f540ac1628fdecb084ffc2746b775a0b926f9c5e913223e9e9d0a2c1b51e5348e6108736f75473c"
    "9359920d461c8ad4f470501be86849c374eb1bcd12abc9c0bac2f32e9cdd535afe3cb4abf9b248d1dda9cbeb8ba7824c"
    "43040be405cd66bf9da908c5cb221bf6dd1211dd90788bc8e4699dece6ce00c12a0284d5b0a4ad30c5b374950819b800"
    "1f17261cf311eaa87d15bb4c4d6a14cee8d1484614183468035f4e12a874a328bde8e7e384c9977fe828ea3568a843e0"
    "6e6bd7d92410e0d52afcfe7fc51c925b";

static const char* const g_rsaKeyBModulusHex =
    "92e4be4ae0f6898647204ce9ab6bfbfc36ed5f6228c0ef29eb692281c8170ec7bfc83b28662e7d72adf19cf9dea51ecf"
    "945e7e9fe1293c42e45af5a315968979a24cab987103c3b443c7dada8fef4faa0df9f72e145fee9327c4e5900a9022a8"
    "79424ffef8c5e71574f74557ff97dc3054715e83434bfd7d90a99a5a090ec4df919008795d72bcbdcbddde5f5f1595fe"
    "4efb97b155968dfb5a8f8f211042c848333df198bafb350d83cefcfbcdab8a2ed55fd4ae31cfe8794982930bbcebe250"
    "0958cff999143c3de31c50490ce72ac82a26fe68ba9e07652aad890ed00d1b7793cd18d4ddb267fb5cc18ace3b57988a"
    "0211ecac0c3b2cc65be2be13f4689301";

static const char* const g_rsaMessage = "RSA PKCS1 v1.5 test message";

static const char* const g_rsaKeyASignatureSha256Hex =
    "61f02f529a3fb7dd4f0c13075695ec73d659175503706666389f229fa73492873cca6eab0d08c4330db7a1158da98f16"
    "3ddca1462cf41458d80a47d21519be3ea43d182c34ceaf63c4e6db2220f0a1e3a39a5a63c3459d4cf8a300127fcd1045"
    "9f6c72002f3a5608b35906232d4701c68c8626c20b5ca855bffd5c71930880fdb345a8de7a37f8a58db24fa13abb9b92"
    "c57593bd4dfcd214ab3d9087eb8ee525634d4ecf6e16027a0dc671ff536ee5b123b0978b524e1e52c0f40e40310071d9"
    "51bb64873906381cdf4f18a5ff2567670b4aeb49a2756bf3b29913b0e1b88717491373e4affcc41215726bbb467d5fca"
    "3f64a6500d63c677f3f0942d2cee9b3a";

static const char* const g_rsaKeyASignatureSha384Hex =
    "71059e7bd08519c17472891fafe423ac8e3eb86d75cdc748081ecb1f0824d96ff67143c89c77ae8e43374f328427bdd8"
    "2ed49132ec451836054d8cc1475d57cce6a22092a770cf1e42f089dfc1ac9c78aa298ae1b83ad70d27983e2c533cfd95"
    "5f852a5e2e69ff6edb57cbec78aa06347ccd46934a0f57154af23b81c8e1afe1ec3bc8c4ceaa04339056a0e15cc1ae21"
    "c265d081dc34b6374aa0cfb62538a72f084893117e3f7aa678d938304530df333e9dbf4783a796f807ab3c729b5eb3ca"
    "e36edbf120a8137466420c7e7572a552f98522d52bb60b8fa6b8efdba04034ccf89822cc3aa129d44d44d13af0c7441f"
    "4689f5cd8df412b2eda426d918331366";

static const char* const g_rsaKeyASignatureSha512Hex =
    "54d81a02224d8cec87b960c04c3a900af5b45ad23ed67b180d4f3248c95d9f1f21997f1c2e938c82d2b50e24baf2dee3"
    "ac1a058a5bda4893f46e7578977be46069ba992efee0828fb7671e23d32ab67262d71077a7b122b0df156a7fee080bd5"
    "43e25f2ff96f89fe90e77e1f8e101009e22d4de404d7bfe505dcc2ac41feb692914e3551eddb69dd5e543cfb384db946"
    "4db7c81b1c55ea00a4036d0db6c1a2bac758a6f8337db0ccd38a891e1f4e7f3e3e3d156eb5983b318537a99e4bbc0793"
    "c48877a638f169701df67ce307ccb28c165d6f976e4eda558c661c12e4918255ab8b9106c0ff5700b4b8b40aa369295b"
    "682af72a74e751bfebb8dc2421556a65";

static const char* const g_rsaCraftedControlHex =
    "61f02f529a3fb7dd4f0c13075695ec73d659175503706666389f229fa73492873cca6eab0d08c4330db7a1158da98f16"
    "3ddca1462cf41458d80a47d21519be3ea43d182c34ceaf63c4e6db2220f0a1e3a39a5a63c3459d4cf8a300127fcd1045"
    "9f6c72002f3a5608b35906232d4701c68c8626c20b5ca855bffd5c71930880fdb345a8de7a37f8a58db24fa13abb9b92"
    "c57593bd4dfcd214ab3d9087eb8ee525634d4ecf6e16027a0dc671ff536ee5b123b0978b524e1e52c0f40e40310071d9"
    "51bb64873906381cdf4f18a5ff2567670b4aeb49a2756bf3b29913b0e1b88717491373e4affcc41215726bbb467d5fca"
    "3f64a6500d63c677f3f0942d2cee9b3a";

static const char* const g_rsaCraftedBlockType2Hex =
    "53d7acce9f4291954e0002cba79fb456656e3586be6b001b53bd9067c6947b228672c98304d7de4d9edda9889b2a0a73"
    "b86c2f8d272287f294b85cee5aab0d845c2833ec662b3f0b45bddfe7f53d9c26d6d308656e1c610ec191ee07c052ef14"
    "b7a9730716ea833c867fde1739d3df0b7b12487439f4f9b0f1e7b03f7ec916c42d23bbe311161d08347e0254ab430662"
    "9df194a87ab9585b5cc651a48ca643f8f93df2938ef806709470107203bbaa73d1578b3fe93432a3d566be84e8578a4a"
    "1ee218a6892389fe61d6aa6471b60dd3e227be64d427676c75d1fb06787969343a97d3e7cdb3981f77514b692e07f92b"
    "efb5b9a7004ebf8c03e6db7857f00293";

static const char* const g_rsaCraftedPaddingByteChangedHex =
    "504cce9a9c3842e55735598128627f48017dea889c609313779366c4effce21d8a403471d6e8bf49efdf600c844b61dc"
    "2fb022d5ecaeabaa15e24f3ead018e5b8d311a180eadf0b2f9d1e85bfb1590980b67cbdc6aeba3db9bd54186a184b101"
    "36c11352246d5f3ec892af5c747c7bff66f05c94ecfe0348bedd08b21755c040737c498cc4fadfde8182ef54956b15cd"
    "3b3a819bbc4398c161ffad2b134173f9fbdec6407c9877c80ef937363883d5819848f9ac9a370b537901a8edf0597375"
    "c026a3c9780292fe28ced7198b7be26731b802636938bb9ace229c622fd98c89323fc8ece767011529ffe57cfcc0f31d"
    "1b3d1c26c8cb82728253c5bcfd3c96b1";

static const char* const g_rsaCraftedNoNullParamsHex =
    "61646426c78128c6194136c5ac7c9938e07dd607c6007fc87a97ccc0e2172d0dc4bfe64ea698b6b223cfbc5da57f89d5"
    "764c3ed093f46d67fbabfed83b63666389c21b36d50c015f636362269e15a9e0c93704f00579f49d7c305ce531df8a62"
    "138900cd7e796e6d0720e2073e31f331e8830c82a0c8bd7f772529f576bc4b52e6cf68462f62f4313361dae67630c240"
    "f5d3134d7279dfefaa3fc60b84c72fc1c74e0ed608a1063ef9bb9093ec6b37a1ffb8353bbb10e9b8125bbc09425db882"
    "b8d7099279b8e519580e38b28eee1bb0d3270f163e40f79f01eaa7450a4cf4e6c764a26d690619d7eab93af85836a824"
    "737fe93c540a82abf0276fc4be55f854";

static const char* const g_rsaCraftedTrailingGarbageHex =
    "8dd997ebf06694b6a26bfdbf75b536549c4183b5a4c3873cb2660c2b74182c82becfa1472ba7962a85c84194f99aa7e0"
    "820ae167b1c6663043ef5b1bacb5749d6af4ddc191b94f0c5fc844a78fb284c5b8d3289f3485ef33c58f4d9528b82f38"
    "12227c90096dcfe4db7a0819617a2b30f85c749f3a728d73fc2320b7d28ce56eb4360b7b4341735bbfbecab8988e8892"
    "7b76da43ec8c04fee87d5d561499c77ac51a6fbd8afd95387b103cf5b4eff43b47280991a489f462e2602d7a45b996fe"
    "0577ab673c3f456ffb19a9ca2da82319f07d372c242148522990a7bf36f708a036881d237ffb19e87ca231904b84ba1f"
    "4bcaabf7ea7359be7a9e6d0f738cc511";

static const char* const g_rsaCraftedShortPaddingHex =
    "78dc31f0709981d694952e4f74f4cc85be97c23431bdf5f0a50b675e426c2255327977a5b405b7267da900f038c4dfe5"
    "9084ea2d88e2d7114f9524ad6f742c803d0b7774459fed26fac8acb68cbd5c6586bb39a24c9a215c9499a3c286b839aa"
    "92c9b68724256914a14c429d4b19a911343877275a0c9dbe7059b4eb3d969a3c302977c7c5ff10eb0fe304e70210799e"
    "6c9c08fcf643202f4501637e8d56bc91be2095468817fcf6d34e49158ae5f20b8913d5dd2beb848125f341fa1758350c"
    "5e8ba9b11ad578732cd295aa920594b2dbcbd189dc83c335771f37f4047a646968e9f71aa62d022369cf3328d2a44dc6"
    "922bd7270a793324176fb3a8f3a42941";

static const char* const g_rsaCraftedLeadingOneHex =
    "1c46ab592a5b8a908c2b7a3b2783ae2e635dcbe783447c4fbe96238fef76d6dfe4a7fe29d5f299d008b8b75d1d2afb2e"
    "a1bd8d5b717f13538bcdb1f938197cd0bbce49b6f9286c1accbf2417f9d62a22f5cfa8b2bff4ee8d760046a144b09b5e"
    "44576043273cca848cada520c9f4a6693b79babe3565c1ecca9b190fca8da370d87b1f268b94cb83c69247e69fd8be1b"
    "ffe835278ddb56698a8e97767ab6cc62a33b46774a3faaf018f658d06ca61080995b991d36cdbd65e679004fced3dcc0"
    "910852b462b47abb8c1a49bd75a0f332d24e4f78e6192f88794c164e4ef7814a7390860a6aea9f824cc4ecbd220249a1"
    "708d7e5ced683694f196c88abbe144b0";

static const char* const g_rsaCraftedNoZeroSeparatorHex =
    "3b29174277d5c348f0977c9b55436bdfdff17386a74bf78f408825a582fa784014043945daa7414322d7c516df8d5d14"
    "2544c9d045b63ea2cdf7a877670a023a03f90ec5bda222bc251b8fdc3602f074a22c168feaf51997c8e561b08169fe72"
    "0366739fd8ae04182fc0d283f579edc5366b44426940767d159c1966e40abcab019eac41f6df23af908384529d3de53d"
    "5c29447568671b2fd3c7129229d654b05bd08a18d034df14f1c7fa4c10067b0edea8f7eae1986a2fddce7d30d7127bef"
    "4230f82abe32810da7b808ea4fb1e4ba159c334a2dc0393cb2ab098a1ec0c5ab302fdd66b17a00d02cc7f281212e0932"
    "9cf62c3d90e5c747d24e713875c81750";

static const char* const g_rsaCraftedWrongDigestLengthHex =
    "3e55db66a03d281a5fb1450598f0cb04e941bc010106028b9f49dc08ef99f90e28059cbc2e0b9564d05de3b41ec48849"
    "63bc4c8310e96e5ddd1c0270f40597dbf914d164e8d0702ee918353441e34e2b462e4b99a62f7b63a991077794e5d51e"
    "a43d05b097845e49cdcb6abda5c87b7c487c251215fd263fb83356c8215110ff092bd162cb3bdedc5cdc0ee955a57c10"
    "8ad8e2196dd91655f994bf9ca264da75182dbd2dda499a2979f1cc5e2a7cf2e5dc842d0545a4aaed6e43281dc99171ee"
    "85e6b25f6a7d69435d2fe50e514616d9dd85a60a33080ae8b678d934ff5cacc3339708348e90f6a83f4c3ddae90ed999"
    "e4986f39e3acfe4ca9778c7cf68732f5";

static const char* const g_rsaKey4096ModulusHex =
    "a46036c592a9eecab03ee886dd5711d6801ab675480450a18b7652ebadb9135ad22bbd219d510eef4ebf0525f4525c93"
    "f854c23a63ae0a6ffb683194338cda44a5d1aa8eb74103c032e5d648cdf64c11ec2574cec9fe09ec192f3071df3e83ca"
    "bffa0b0b30279f5dcff05f9f62cf33db1fd59df2535222efaf04a211086d1c1945984413426246f5cbe53b9d61cc9a02"
    "08c44f257b17db33944b844cab84b4489f11459b28988f4e3ab707a329a955d7a57f0a0bdc0098de2daedcaa0f8039a9"
    "8dffaa05008b8c1a43e5ec31ab2f316b309d89576f1e9b4fdb2ace48a571a26dedb98c403dd2b2c627b712585b353f6f"
    "c628dd6d9b676c5c3ecbe52bdb1a1b54270f166f2134e4cfb3506efc8243f8994330befe923d19b5e973739337d7c557"
    "94aa8bd76295124d5a30fd7e4c4e5461a7afc703f255fd7b8b597f33ffbd8003e7bed7c2f7122378aec9ff37390b9d16"
    "a471c7bd9ba1ad78abd0ed721734f97f8475ac8680970f13542e72f59b9624e50455aa0e8f461f4d6604a9be35e43ac2"
    "0cb9078d9609687019d441e27aa0b0831ac1048d4a4ff676aff3063f7918ac5c4adbea24263776fa9a499f3408dd156f"
    "c50fc1c7421dd18b810b8c6f29c906569924ff7c99f2b9a847e3f6d7bf3ce32f65787dd63e88027fcdf806ef51608570"
    "ff4e114ade5e3f52b32069663e228f57b7c527a94221f0d56beba5358d536689";

static const char* const g_rsaKey4096Signature256Hex =
    "08a71ef7cdf1ee8235548a92e34c88d7161fbd25a09e26cb29768fec5b349fd5bbde52a31a487eb0025c460d23cf5261"
    "f598f5dc867087a71edd69b0907e93f576fc14e16c7f2b63682b30ef8a1578bbf926c0d928273f65cd3bbfdbb5820714"
    "84b11e5c26bfcffae35d78be3004a473d2b46083a6c97fce26a46aee23a1574e95e124904be875b1f68fe92a0e2a7696"
    "b362680a5c003a765ae44d0100dc3849d7f198f8cf6ad9c76f3e6daebe9704170e0c6b90f97559ee3ab7464f3d5c2503"
    "cb8df8876759e63ae803023b08a28c6ec16c760880b69ef1059f03fd0fd2bc041f80ff19e03d857421529e65a2cdb402"
    "fc55ada3644b77e275a7585a9c791bde165b535d3f674a436f1b22b198371e52389b964650036ef84ca3a2366b9f6fe3"
    "ef9272c5e3f3f2c4336c601ca12bb6b0b1f7b6752eb9b1d2769e987f7583e76081eb386d2bd2d575c470611293e7c4a6"
    "3b7b368a49db66c3f50b1c2fca09b7d496c288a324e9623d7287a7f239f4663077a26e9ac79e5fe5df0628f0e6e45127"
    "19886cf2ec551d93c05a708536a89d19ceb529d05427b0c8307b9f27076c3ad62b122c7cb8fd012ab35042e7338a4313"
    "2e7e67c1961e220174acfb7425de8875e59b63fbaed8e43282756836eeea9ad0bcb69cb9c2a682795bbf1cf1371e2788"
    "516ca375a40c11c6c1e4bbe1297df34cb44858ae1e73914634f32cb9e93fe492";

static const char* const g_rsaKey3072ModulusHex =
    "c67bb43cc44e4584b84163027b12c119500cbd7cacc1c5b9ba4c6e1f19bafc0482159e72e4a6f34003ba4dccafc06803"
    "dce966336db935700358e4873b973b7281074371c243d4ce570e5aa50fb479cee23dfdc019aaa90893b31fb64a79c212"
    "29d62a0d1df36d5cf3cbe42a27ee2722edd8ae2f07d9934955ed526fd1b721a962f203bf042517b8f7787042c32aeee3"
    "e89ae10eb879a2cf4b2c1108e52ec442c7737fbafa60ef7817d29be5f615a6be211aed9220e3b9c330ee6912d25d964c"
    "15b2aaa5d989e465ff4b690fe68a10a162af69320a40b73b61019ce4f72a24ffdb1e1db31b5b99ad72f7147a0c77b46a"
    "d8607133137c2b41c1119a88c3c11cdfcfb1d2ffaa91a26e6e68b045dc4e57c82bdff6854c926367c6db0794b6faec63"
    "641aeb6aa5115a7e6b0a18cb74b3708f60bb019341c898a2432ca9493f4597a635285f33cb8a02c4ae6272b997948d7e"
    "39eb25d6930850cafecd3a4e30c3471dab938ddfe453540bb74a240e57f88137feb1190c7c412c22a21695d466e62b45";

static const char* const g_rsaKey3072Signature384Hex =
    "bbf3bc8be7cccb10ca27b8e8294751cd3fbf8095ee23bd3dbc03621e01b000847d4ef1293b0505a43b6664199a31220c"
    "4ec15b5e39f6b131e01f9c88fd5e59030f64c1fbe1c94f0b6c410c8575a6c6d1be1cf6a478e792955c2ce70a61c28f9f"
    "bbb9f74c47495d7325db99aa8899b956bcc223d91d14c9c866d50dae61c13b24ffe921dc727d4285d482134a88ad7d18"
    "d81b64ac18ac17f80160d76de174961a197824f34e3bdaf76097d15058b222badf62b80d5c0b163843f0cc13421b781f"
    "c8f97b47d980fbee9bb24bb5be58abd98e23c3c89cc453d8d79311a9a18d80b654df75bc69d9b028965a116e8e0223b2"
    "4803a3dcab587d9b007d189b01a2f3aac85ca96d6a9f47e8a6dfb40b7241bed3197e38c75d3f2cebc7fbb2a728d6b485"
    "428366531b33a9b90143c74149c16a628d25c1b6977ac2561dd4d28dc4669acf9e829962c3852c49680d96ab813cc382"
    "74539090012067daf9169c726bb046f95e5c8c6a7bfc94ddc8ae654983f0ec9ebc12c91fcc9e5257b5284433f562bada";

static const char* const g_rsaKey1024ModulusHex =
    "f35c2bab8058e4119f7f1a6357aedce86bfba654b1271c4b5a127bb432be16df0843cf88d57a8876577df5fdfc72743c"
    "b3c8b686696c43c39271a503c61addf5f843904b1dc92304a73e98efdc671425fe433f788e0a4b4d706f631cece21021"
    "16a87f5dd55f4d864c035dd4ce528c107b741f18b009967940624132fcee7807";

static const char* const g_rsaKey1024Signature256Hex =
    "20319bc8f0d77fe4cbdcaa066a14d4d1b9754644fcbcbcb75e55ff33c7ee834a244dc8290efa076f5a2c45d435526534"
    "ce3f4ba9599767caaf6f62050070842dc1de62ea1391a2a76b13ea106ef92574df5a28d30109700aac0d5392bfc61fc6"
    "cf84735f677b42c6a0e4b4eb75694e49bb05e03a2a99db2bec38f5f85529025a";

static const char* const g_googleStyleJwt = "eyJhbGciOiJSUzI1NiIsImtpZCI6IjFmOGIwYTdjNWQyZTRhOWI4YzNkNmU3ZjAxMjM0NTY3ODkwYWJjZGUiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiIxMjM0NTY3ODkwLWFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6MDEyMzQ1LmFwcHMuZ29vZ2xldXNlcmNvbnRlbnQuY29tIiwiYXVkIjoiMTIzNDU2Nzg5MC1hYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAxMjM0NS5hcHBzLmdvb2dsZXVzZXJjb250ZW50LmNvbSIsInN1YiI6IjExMDE2OTQ4NDQ3NDM4NjI3NjMzNCIsImVtYWlsIjoicGxheWVyQGV4YW1wbGUuY29tIiwiZW1haWxfdmVyaWZpZWQiOnRydWUsIm5vbmNlIjoibi0wUzZfV3pBMk1qIiwibmFtZSI6IlRlc3QgUGxheWVyIiwiaWF0IjoxNzAwMDAwMDAwLCJleHAiOjE3MDAwMDM2MDB9.fNve9k8Q84E20n84EdAEKWWOR7RrJB2U5ri6baImg9FNY27PINEsQcPdZofd0c_xOOlRT8uex11ubfjC3ikknJcAltsIkl6GII3-qAaVE_b5JX-lZj-lnUO56IEpSxaIwM9qw3-CYBJZI77ItHe27iggA73BxBaqI0m5qhMBkF2bzHv5A7_COCLzuEzJpmnmtWg0IKaNcx5UBcmi6UZWFd6r6ZWaP0g13Eae0Dq9igEkZV5wGb-vh8afz9kb8ODxRIimzfBYId6v82Bz0L7hFo-5DTmMb8hEp4bVX-V5v1vx1Qyu5L7uXrIRRufd0YysBg3CQld1-GuV5qt9IO8gUg";

static const char* const g_googleStyleJwkModulus = "pPrquXRr0GCeqL4Qa2MYimYLSGADm6Cy9IkXXCBrvnxfvrjw8pmEfKJXKm5T3mAUQTiYI3iJFeV_VArBYo_eywhP_CdGt3WguSb5xekTIj6enQosG1HlNI5hCHNvdUc8k1mSDUYcitT0cFAb6GhJw3TrG80Sq8nAusLzLpzdU1r-PLSr-bJI0d2py-uLp4JMQwQL5AXNZr-dqQjFyyIb9t0SEd2QeIvI5Gmd7ObOAMEqAoTVsKStMMWzdJUIGbgAHxcmHPMR6qh9FbtMTWoUzujRSEYUGDRoA19OEqh0oyi96OfjhMmXf-go6jVoqEPgbmvX2SQQ4NUq_P5_xRySWw";

static const char* const g_rfc7515A2Modulus = "ofgWCuLjybRlzo0tZWJjNiuSfb4p4fAkd_wWJcyQoTbji9k0l8W26mPddxHmfHQp-Vaw-4qPCJrcS2mJPMEzP1Pt0Bm4d4QlL-yRT-SFd2lZS-pCgNMsD1W_YpRPEwOWvG6b32690r2jZ47soMZo9wGzjb_7OMg0LOL-bSf63kpaSHSXndS5z5rexMdbBYUsLA9e-KXBdQOS-UTo7WTBEMa2R2CapHg665xsmtdVMTBQY4uDZlxvb3qCo5ZwKh9kG4LT6_I5IhlJH7aGhyxXFvUK-DWNmoudF8NAco9_h9iaGNj8q2ethFkMLs91kzk2PAcDTW9gb54h4FRWyuXpoQ";

static const char* const g_rfc7515A2Signature = "cC4hiUPoj9Eetdgtv3hF80EGrhuB__dzERat0XF9g2VtQgr9PJbu3XOiZj5RZmh7AAuHIm4Bh-0Qc_lF5YKt_O8W2Fp5jujGbds9uJdbF9CUAr7t1dnZcAcQjbKBYNX4BAynRFdiuB--f_nZLgrnbyTyWzO75vRK5h6xBArLIARNPvkSjtQBMHlb1L07Qe7K0GarZRmB_eSN9383LcOLn6_dO--xi12jzDwusC-eOkHWEsqtFZESc6BfI7noOPqvhJ1phCnvWh6IeYI2w9QOYEUipUTI8np6LbgGY9Fs98rqVt5AXLIhWkWywlVmtVrBp0igcN_IoypGlUPQGe77Rw";

/** Modular exponentiation: modulus, base, exponent and result, hexadecimal text without a fixed width */

static const char* const g_modExp0ModulusHex =
    "f2a74de452e6b439";

static const char* const g_modExp0BaseHex =
    "6513270e269e0d37";

static const char* const g_modExp0ExponentHex =
    "c5c7fd0a6a3a451";

static const char* const g_modExp0ResultHex =
    "d46d0f5e423acf54";

static const char* const g_modExp1ModulusHex =
    "31818e811892f902bd23f0824128b2f33";

static const char* const g_modExp1BaseHex =
    "81e74ef5e8e25d940ed904759531985d";

static const char* const g_modExp1ExponentHex =
    "6b0d549b6f03675a1600a35a099950d9";

static const char* const g_modExp1ResultHex =
    "1354b598eaeada10d19686d3f39c3bac6";

static const char* const g_modExp2ModulusHex =
    "cef8aa38922766581e27a1c08a6a63ec24ede6a46b4cb2424a23d5962217beaddbc496cb8e81973e0becd7b03898d190"
    "f9ebdacc0cb1e29c658cda1495e60af593bd04cf0fd630f1f29d0da9953f48f1a09f76b5a170b33839263059f28c105d"
    "1fb17c2390c192cfd3ac94af0f21ddb66cad4a268d116ece1738f7d93d9c1725";

static const char* const g_modExp2BaseHex =
    "cb5c74273f98e2774cbd87ad5c90a9587403e430ec66a78795e761d17731af10506bf2efc6f877186d76b07e881ed162"
    "ae2eb1547f15052434b9b5df9e7769b10f4205b4907a70c31012f037b64ce4228c38fb2918f135d25f557203301850c5"
    "a38fd547923a736994e3bf911a61dbe22e44158bae97ba94d0eda82f8f6d0558";

static const char* const g_modExp2ExponentHex =
    "c3baea9e13deef86ab1031d0f646e1f40a097c976bf46c697d2caf82eeeacbe226e875555790f82ec1d3fcff2a3af4d4"
    "6b0a18e8830e07bc1e398f1012bd4acefaecbd389be4bcfc49b64a0872e6cc3ababced2057ee05cde00902c77ebff206"
    "867347214cdd2055930d6eaf14f4733f3e7d1bfbc7a2ea20b2f14c942e05319b";

static const char* const g_modExp2ResultHex =
    "a6c47213a2f922b48a8c524e79a3db1823d43e55f110c0f9d84a7b1f261f1866f640dd683d0bd6831755fcbcc3b69058"
    "1f3d3cdff84fc96509a26a16d698301b915dc074f4c6eb5f810ee8c39934a6786e896d2b09db6c2045074b64063c145f"
    "f677f7623eaac9e3cde4db68694e03cb80744bfed1890c80d95177e9afd9b6d5";

static const char* const g_modExp3ModulusHex =
    "8ca8181166d2287672fdf2022a96fb1a14a0f9e77f1b103cdf1582b0eab477d26415479c65dc9f503f63af83bd0561e6"
    "211c70cf49952399c4aaeac137dc76fb0f17a3007e62aa0a1df9fd789c6539382b0537e65affb2297631a992f0ce5835"
    "05c6af0758d5563dab2cd31ee315128862c33a4fb774eb5248db40af72158370d269a9a5ae658f33fe3b890b93f448b3"
    "a5aa3c814f426dcbb394fb36bb2d420f0f88080b10a3d6b2aa05e11ab2715945795e8229451abd81f1d69ed617f5e837"
    "d70820fe119a72d174c9df6acc011cdd9474031b7f26144b98289fcd59a54a7bb1fee08f571242425051c1ccd17f9aca"
    "e01f5057ca02135e92b1d3f28ede0d7b";

static const char* const g_modExp3BaseHex =
    "1a81682c64e50cad66237a0465e7e4236472f1a38f2c6ec8cc4169a3ae3a2b7fdfe01893f3aed0b6c7ac1491def88334"
    "e647cb8f74e69a5d0dd27a65bd628881ad1b72dba7abe1c29e1a8ef4f341e07a83f73f16dbf4a8b2b0c4312d20203626"
    "f3fe39c0519088f590fbbd119c1caaf75e8766ed88daf4016b4013ef254b0c4e010c4759482c9cbc43435cc52eae05cf"
    "96d0cc5fd4c28c2e7c26847f0316909e3bbbe9eaa8948c893b61867626bb7dbd2d1c9af0153e7c2a26a2c0bd3b1287ff"
    "f52ddf5d616499c9e25a7605aec6f0245bd86d40fc891b4a6a50df4db4d66a3a47469a4d8cdb305fdd2e16096e36aab0"
    "d1bc52d9230d977ee22571594720771f";

static const char* const g_modExp3ExponentHex =
    "10001";

static const char* const g_modExp3ResultHex =
    "2f3e35d58075025ce36e63f92b84c8c0710cdb387e2684d316bc89a91fdf9ffa8e0910c0a97fecef7c0b6470e27f580d"
    "b8a49ce5711b0993f3083ffcc53537cf6dca4f4459730fd3c7e3129acf9f7a283488bfff4411892870c7fbba3a388a00"
    "a0735a5dcb10e59c1bb29ccecfd90015508bff8264c7fbf39ec2d21c9415bf262ee647871ebb3c1282ed4adf27ffda62"
    "7463d064351076f4adb0ef0009d33cc4ef70aecd575a5da2d31d93c6044caa2d62e9dbddc4f45f06048db1f80004cb87"
    "340535e1b474a0adf028d1f09885a1b023f62d610d9cb3ed2080ebf90d4953de70e9799a86411eaf818d3c6f183d7f91"
    "89281f12e7d8e316d5b86a645c0a0450";

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTRSAVECTORS_H ? */
