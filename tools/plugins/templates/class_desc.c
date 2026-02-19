ORCA_API struct ClassDesc _${cname} = {
	.ClassName = "${cname}",
	.DefaultName = "${default_name}",
	.ContentType = "${content_type}",
	.Xmlns = "${xmlns}",
	.ParentClasses = {${parents_str}},
	.ClassID = ID_${cname},
	.ClassSize = sizeof(struct ${cname}),
	.Properties = ${cname}Properties,
	.ObjProc = ${cname}Proc,
	.Defaults = &${cname}Defaults,
	.NumProperties = k${cname}NumProperties,
};
