# see https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html
if(ENABLE_BF16)
  set(OCV_FLAGS "${OCV_FLAGS}+bf16")
endif()
if(ENABLE_DOTPROD)
  set(OCV_FLAGS "${OCV_FLAGS}+dotprod")
endif()
if(ENABLE_FP16)
  set(OCV_FLAGS "${OCV_FLAGS}+fp16")
endif()
if(OCV_FLAGS)
  set(OCV_FLAGS "-march=armv8.2-a${OCV_FLAGS}")
endif()
