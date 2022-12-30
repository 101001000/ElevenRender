
; __CLANG_OFFLOAD_BUNDLE____START__ sycl-spir64-unknown-unknown
; ModuleID = 'llvmtest.cpp'
source_filename = "llvmtest.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn
define dso_local spir_func noundef float @_Z6ir_sumff(float noundef %a, float noundef %b) local_unnamed_addr #0 {
entry:
  %add = fadd float %a, %b
  ret float %add
}

declare dso_local spir_func i32 @_Z18__spirv_ocl_printfPU3AS2Kcz(i8 addrspace(2)*, ...)

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "sycl-module-id"="llvmtest.cpp" }

!llvm.dependent-libraries = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.spir.version = !{!3}
!spirv.Source = !{!4}
!llvm.ident = !{!5}

!0 = !{!"libcpmt"}
!1 = !{i32 1, !"wchar_size", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{i32 1, i32 2}
!4 = !{i32 4, i32 100000}
!5 = !{!"clang version 15.0.0 (https://github.com/intel/llvm 5352b423f8913fce44e8276de68a39ffdb190bdc)"}

; __CLANG_OFFLOAD_BUNDLE____END__ sycl-spir64-unknown-unknown

; __CLANG_OFFLOAD_BUNDLE____START__ host-x86_64-pc-windows-msvc
; ModuleID = 'C:\Users\Kike\AppData\Local\Temp\llvmtest-f03170.cpp'
source_filename = "C:\\Users\\Kike\\AppData\\Local\\Temp\\llvmtest-f03170.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.34.31933"

%struct.fenv_t = type { i32, i32 }

$_Fenv1 = comdat any

@_Fenv1 = weak_odr dso_local constant %struct.fenv_t { i32 1056964671, i32 0 }, comdat, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef float @"?ir_sum@@YAMMM@Z"(float noundef %a, float noundef %b) #0 {
entry:
  %b.addr = alloca float, align 4
  %a.addr = alloca float, align 4
  store float %b, float* %b.addr, align 4
  store float %a, float* %a.addr, align 4
  %0 = load float, float* %a.addr, align 4
  %1 = load float, float* %b.addr, align 4
  %add = fadd float %0, %1
  ret float %add
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6}
!llvm.module.flags = !{!7, !8, !9}
!llvm.ident = !{!10}

!0 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1900\22"}
!1 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!2 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MD_DynamicRelease\22"}
!3 = !{!"/DEFAULTLIB:msvcprt.lib"}
!4 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!5 = !{!"/FAILIFMISMATCH:\22annotate_string=0\22"}
!6 = !{!"/FAILIFMISMATCH:\22annotate_vector=0\22"}
!7 = !{i32 1, !"wchar_size", i32 2}
!8 = !{i32 7, !"PIC Level", i32 2}
!9 = !{i32 7, !"uwtable", i32 2}
!10 = !{!"clang version 15.0.0 (https://github.com/intel/llvm 5352b423f8913fce44e8276de68a39ffdb190bdc)"}

; __CLANG_OFFLOAD_BUNDLE____END__ host-x86_64-pc-windows-msvc
