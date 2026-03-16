#ifndef __DARKREIGN_PROPERTIES_H__
#define __DARKREIGN_PROPERTIES_H__
// FtgPackage
#define ID_FtgPackage 0xcea32f40
#define GetFtgPackage(_P) ((struct FtgPackage*)((_P)?OBJ_GetComponent(_P,ID_FtgPackage):NULL))
#define FtgPackage_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_FtgPackage,sizeof(struct FtgPackage),_N)
#define ID_FtgPackage_FileName 0x3ba83167 // FtgPackage.FileName
enum FtgPackageProperties {
	kFtgPackageFileName,
	kFtgPackageNumProperties	
};
#endif