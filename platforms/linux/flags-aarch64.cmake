# see https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html
function(ocv_set_platform_flags VAR)
  cmake_parse_arguments(arg "APPEND" "" "" ${ARGN})
  if(ENABLE_BF16)
    set(flags "${flags}+bf16")
  endif()
  if(ENABLE_DOTPROD)
    set(flags "${flags}+dotprod")
  endif()
  if(ENABLE_FP16)
    set(flags "${flags}+fp16")
  endif()
  if(flags)
    set(flags "-march=armv8.2-a${flags}")
    if(arg_APPEND)
      set(${VAR} "${${VAR}} ${flags}" PARENT_SCOPE)
    else()
      set(${VAR} "${flags}" PARENT_SCOPE)
    endif()
  endif()
endfunction()
