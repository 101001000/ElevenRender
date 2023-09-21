; ModuleID = 'red.cpp'
source_filename = "red.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.34.31933"

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @"?shade@@YAXMMMAEAM00@Z"(float noundef %u, float noundef %v, float noundef %f, ptr noundef nonnull align 4 dereferenceable(4) %Cr, ptr noundef nonnull align 4 dereferenceable(4) %Cg, ptr noundef nonnull align 4 dereferenceable(4) %Cb) #0 {
entry:
  
  store i32 0, i32* %Cr
  store i32 0, i32* %Cg
  store i32 0, i32* %Cb

  store i32 1, i32* %Cr
  store i32 0, i32* %Cg
  store i32 0, i32* %Cb

  %Crl = load i32, i32* %Cr
  %Cgl = load i32, i32* %Cg
  %Cbl = load i32, i32* %Cb
  
  %Crf = sitofp i32 %Crl to float
  %Cgf = sitofp i32 %Cgl to float
  %Cbf = sitofp i32 %Cbl to float
    
  store float %Crf, float* %Cr
  store float %Cgf, float* %Cg
  store float %Cbf, float* %Cb

	
  ret void
}

attributes #0 = { mustprogress noinline optnone uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"MaxTLSAlign", i32 65536}
!4 = !{!"clang version 18.0.0 (https://github.com/intel/llvm dbd9b67cd5c097b41f20c8a61f37c711888f2a20)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
