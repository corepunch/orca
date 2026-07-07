#ifndef ORCA_CODEGEN_H
#define ORCA_CODEGEN_H

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }
#define UIDATA_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) DECL(SHORT, CLASS, NAME, FIELD, TYPE, ##__VA_ARGS__)
#define UIDATA_ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE, ##__VA_ARGS__)
#define SHORTHAND_TARGET(TYPE, FIELD, CLASS, PROPERTY, BIT) { .Name=#FIELD, .PropertyID=ID_##CLASS##_##PROPERTY, .Offset=offsetof(struct TYPE, FIELD), .PresentBit=BIT }
#define SHORTHAND(CLASS, NAME, TYPE_NAME, TYPE, SHORT) { .Name=#NAME, .Category=#CLASS, .TypeString=TYPE_NAME, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .StructSize=sizeof(struct TYPE), .Targets=CLASS##NAME##ShorthandTargets, .NumTargets=sizeof(CLASS##NAME##ShorthandTargets) / sizeof(*CLASS##NAME##ShorthandTargets) }

#define ENUM(NAME, ...) \
ORCA_API const char *_##NAME[] = {__VA_ARGS__, NULL}; \
const char *NAME##ToString(enum NAME value) { \
	return (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value]); \
}

#define STRUCT(NAME, EXPORT) \
static struct StructDesc _##NAME##_StructDesc = { \
	.StructName = #EXPORT, \
	.Properties = _##NAME, \
	.NumProperties = sizeof(_##NAME) / sizeof(*_##NAME), \
	.StructSize = sizeof(struct NAME), \
}; \
void register_##NAME##_struct(void) { OBJ_RegisterStructDesc(&_##NAME##_StructDesc); }

#define STRUCT_PARSER(NAME, EXPORT, PARSER) \
static struct StructDesc _##NAME##_StructDesc = { \
	.StructName = #EXPORT, \
	.Properties = _##NAME, \
	.Parser = PARSER, \
	.NumProperties = sizeof(_##NAME) / sizeof(*_##NAME), \
	.StructSize = sizeof(struct NAME), \
}; \
void register_##NAME##_struct(void) { OBJ_RegisterStructDesc(&_##NAME##_StructDesc); }

#define REGISTER_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, .DefaultName = #NAME, .ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, .ClassID = ID_##NAME, .ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, .Shorthands = NAME##Shorthands, \
	.ObjProc = NAME##Proc, .Defaults = &NAME##Defaults, \
	.NumProperties = sizeof(NAME##Properties) / sizeof(*NAME##Properties), \
	.NumShorthands = sizeof(NAME##Shorthands) / sizeof(*NAME##Shorthands), \
};

#define REGISTER_MESSAGE_ACTION(NAME, XML_NAME, NUM_PROPS, PROPS) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = XML_NAME, \
	.DefaultName = XML_NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { ID_SendMessageAction, 0 }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = PROPS, \
	.Shorthands = NULL, \
	.ObjProc = NULL, \
	.Defaults = NULL, \
	.NumProperties = NUM_PROPS, \
	.NumShorthands = 0, \
};

#endif
