
.set X86_64, 1
.set VK_DEBUG_REPORT_ERROR_BIT_EXT   , 8       # The numerical value of the enum value 'VK_DEBUG_REPORT_ERROR_BIT_EXT'
.set PTR_SIZE                        , 8       # The size of a pointer
.set HASH_SIZE                       , 32      # The size of a 'loader_dispatch_hash_entry' struct
.set HASH_OFFSET_INSTANCE            , 8128    # The offset of 'phys_dev_ext_disp_hash' within a 'loader_instance' struct
.set PHYS_DEV_OFFSET_INST_DISPATCH   , 656     # The offset of 'phys_dev_ext' within in 'loader_instance_dispatch_table' struct
.set PHYS_DEV_OFFSET_PHYS_DEV_TRAMP  , 16      # The offset of 'phys_dev' within a 'loader_physical_device_tramp' struct
.set ICD_TERM_OFFSET_PHYS_DEV_TERM   , 8       # The offset of 'this_icd_term' within a 'loader_physical_device_term' struct
.set PHYS_DEV_OFFSET_PHYS_DEV_TERM   , 24      # The offset of 'phys_dev' within a 'loader_physical_device_term' struct
.set INSTANCE_OFFSET_ICD_TERM        , 8       # The offset of 'this_instance' within a 'loader_icd_term' struct
.set DISPATCH_OFFSET_ICD_TERM        , 736     # The offset of 'phys_dev_ext' within a 'loader_icd_term' struct
.set FUNC_NAME_OFFSET_HASH           , 0       # The offset of 'func_name' within a 'loader_dispatch_hash_entry' struct
.set EXT_OFFSET_DEVICE_DISPATCH      , 2136    # The offset of 'ext_dispatch' within a 'loader_dev_dispatch_table' struct
