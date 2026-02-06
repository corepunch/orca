#define ID_PropertyType 0xe81fb7a0
#define GetPropertyType(_P)((struct PropertyType*)((_P)?OBJ_GetComponent(_P,ID_PropertyType):NULL))
#define PropertyType_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PropertyType,sizeof(struct PropertyType),_N)
enum PropertyTypeProperties {
	kPropertyTypeCategory,
	kPropertyTypeDataType,
	kPropertyTypeDefaultValue,
	kPropertyTypeTargetType,
	kPropertyTypeAffectLayout,
	kPropertyTypeAffectRender,
	kPropertyTypeIsReadOnly,
	kPropertyTypeIsHidden,
	kPropertyTypeIsInherited,
	kPropertyTypeKey,
	kPropertyTypeValue,
	kPropertyTypeStep,
	kPropertyTypeUpperBound,
	kPropertyTypeLowerBound,
	kPropertyTypeNumProperties
};

#define ID_PropertyType_Category 0x9c5f73a2
#define ID_PropertyType_DataType 0xb7998b3a
#define ID_PropertyType_DefaultValue 0x29fd6eac
#define ID_PropertyType_TargetType 0x95c12bb3
#define ID_PropertyType_AffectLayout 0x94a7dcc9
#define ID_PropertyType_AffectRender 0x0f8c1287
#define ID_PropertyType_IsReadOnly 0xbac951a0
#define ID_PropertyType_IsHidden 0xb745ec16
#define ID_PropertyType_IsInherited 0x96394f6c
#define ID_PropertyType_Key 0xd2f88821
#define ID_PropertyType_Value 0x6d2e8453
#define ID_PropertyType_Step 0x01511768
#define ID_PropertyType_UpperBound 0xdd11f20e
#define ID_PropertyType_LowerBound 0x15c97825
#define ID_PropertyEnumValue 0xc3baa44a
#define GetPropertyEnumValue(_P)((struct PropertyEnumValue*)((_P)?OBJ_GetComponent(_P,ID_PropertyEnumValue):NULL))
#define PropertyEnumValue_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PropertyEnumValue,sizeof(struct PropertyEnumValue),_N)
enum PropertyEnumValueProperties {
	kPropertyEnumValueValue,
	kPropertyEnumValueNumProperties
};

#define ID_PropertyEnumValue_Value 0xe5df7e25
