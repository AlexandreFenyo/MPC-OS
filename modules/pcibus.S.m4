ifdef(`_SMP_SUPPORT', `',
`	.stabs "__pcibus_set"       , 3, 0, 0, _PCIBUS_SET # N_ABS | N_EXT')
	.stabs "__vm_map_clip_start", 3, 0, 0, _CLIP_START # N_ABS | N_EXT
	.stabs "__vm_map_clip_end"  , 3, 0, 0, _CLIP_END   # N_ABS | N_EXT
	.stabs "_nselcoll"          , 3, 0, 0, NSELCOLL    # N_ABS | N_EXT
