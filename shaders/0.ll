; ModuleID = 'shader.cpp'
source_filename = "shader.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.34.31933"

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @"?shade@@YAXMMMMMMMMMMMMMMAEAM00@Z"(float noundef %Px, float noundef %Py, float noundef %Pz, float noundef %Ix, float noundef %Iy, float noundef %Iz, float noundef %Nx, float noundef %Ny, float noundef %Nz, float noundef %Ngx, float noundef %Ngy, float noundef %Ngz, float noundef %u, float noundef %v, float* noundef nonnull align 4 dereferenceable(4) %Cr, float* noundef nonnull align 4 dereferenceable(4) %Cg, float* noundef nonnull align 4 dereferenceable(4) %Cb) #0 {
entry:
  %retval.i312 = alloca float, align 4
  %x.addr.i313 = alloca float, align 4
  %sum.i314 = alloca float, align 4
  %term.i315 = alloca float, align 4
  %term_squared.i = alloca float, align 4
  %numerator.i = alloca float, align 4
  %n.i = alloca i32, align 4
  %retval.i.i267 = alloca float, align 4
  %t.addr.i18.i268 = alloca float, align 4
  %t.addr.i11.i269 = alloca float, align 4
  %t.addr.i.i270 = alloca float, align 4
  %x.addr.i271 = alloca float, align 4
  %tp.i272 = alloca float, align 4
  %retval.i.i222 = alloca float, align 4
  %t.addr.i18.i223 = alloca float, align 4
  %t.addr.i11.i224 = alloca float, align 4
  %t.addr.i.i225 = alloca float, align 4
  %x.addr.i226 = alloca float, align 4
  %tp.i227 = alloca float, align 4
  %retval.i.i177 = alloca float, align 4
  %t.addr.i18.i178 = alloca float, align 4
  %t.addr.i11.i179 = alloca float, align 4
  %t.addr.i.i180 = alloca float, align 4
  %x.addr.i181 = alloca float, align 4
  %tp.i182 = alloca float, align 4
  %retval.i.i = alloca float, align 4
  %t.addr.i18.i = alloca float, align 4
  %t.addr.i11.i = alloca float, align 4
  %t.addr.i.i = alloca float, align 4
  %x.addr.i170 = alloca float, align 4
  %tp.i = alloca float, align 4
  %retval.i143 = alloca float, align 4
  %value.addr.i144 = alloca float, align 4
  %approx.i145 = alloca float, align 4
  %prev_approx.i146 = alloca float, align 4
  %retval.i116 = alloca float, align 4
  %value.addr.i117 = alloca float, align 4
  %approx.i118 = alloca float, align 4
  %prev_approx.i119 = alloca float, align 4
  %retval.i104 = alloca float, align 4
  %value.addr.i = alloca float, align 4
  %approx.i = alloca float, align 4
  %prev_approx.i = alloca float, align 4
  %retval.i86 = alloca float, align 4
  %t.addr.i87 = alloca float, align 4
  %retval.i75 = alloca float, align 4
  %t.addr.i76 = alloca float, align 4
  %x.addr.i.i = alloca float, align 4
  %N.i.i = alloca i32, align 4
  %sum.i.i = alloca float, align 4
  %term.i.i = alloca float, align 4
  %i.i.i = alloca i32, align 4
  %retval.i = alloca float, align 4
  %exponent.addr.i = alloca float, align 4
  %base.addr.i = alloca float, align 4
  %t.addr.i70 = alloca float, align 4
  %x.addr.i66 = alloca float, align 4
  %oz.addr.i = alloca float*, align 8
  %oy.addr.i = alloca float*, align 8
  %ox.addr.i = alloca float*, align 8
  %t.addr.i61 = alloca float, align 4
  %x.addr.i = alloca float, align 4
  %N.i = alloca i32, align 4
  %sum.i = alloca float, align 4
  %term.i = alloca float, align 4
  %i.i = alloca i32, align 4
  %b.addr.i53 = alloca float, align 4
  %a.addr.i54 = alloca float, align 4
  %b.addr.i47 = alloca float, align 4
  %a.addr.i48 = alloca float, align 4
  %b.addr.i = alloca float, align 4
  %a.addr.i = alloca float, align 4
  %t.addr.i43 = alloca float, align 4
  %t.addr.i = alloca float, align 4
  %Cb.addr = alloca float*, align 8
  %Cg.addr = alloca float*, align 8
  %Cr.addr = alloca float*, align 8
  %v.addr = alloca float, align 4
  %u.addr = alloca float, align 4
  %Ngz.addr = alloca float, align 4
  %Ngy.addr = alloca float, align 4
  %Ngx.addr = alloca float, align 4
  %Nz.addr = alloca float, align 4
  %Ny.addr = alloca float, align 4
  %Nx.addr = alloca float, align 4
  %Iz.addr = alloca float, align 4
  %Iy.addr = alloca float, align 4
  %Ix.addr = alloca float, align 4
  %Pz.addr = alloca float, align 4
  %Py.addr = alloca float, align 4
  %Px.addr = alloca float, align 4
  %t = alloca float, align 4
  %u1 = alloca float, align 4
  %v1 = alloca float, align 4
  %i = alloca float, align 4
  %d = alloca float, align 4
  %colx = alloca float, align 4
  %coly = alloca float, align 4
  %colz = alloca float, align 4
  store float* %Cb, float** %Cb.addr, align 8
  store float* %Cg, float** %Cg.addr, align 8
  store float* %Cr, float** %Cr.addr, align 8
  store float %v, float* %v.addr, align 4
  store float %u, float* %u.addr, align 4
  store float %Ngz, float* %Ngz.addr, align 4
  store float %Ngy, float* %Ngy.addr, align 4
  store float %Ngx, float* %Ngx.addr, align 4
  store float %Nz, float* %Nz.addr, align 4
  store float %Ny, float* %Ny.addr, align 4
  store float %Nx, float* %Nx.addr, align 4
  store float %Iz, float* %Iz.addr, align 4
  store float %Iy, float* %Iy.addr, align 4
  store float %Ix, float* %Ix.addr, align 4
  store float %Pz, float* %Pz.addr, align 4
  store float %Py, float* %Py.addr, align 4
  store float %Px, float* %Px.addr, align 4
  store float 1.000000e+00, float* %t, align 4
  %0 = load float, float* %u.addr, align 4
  store float %0, float* %u1, align 4
  %1 = load float, float* %v.addr, align 4
  store float %1, float* %v1, align 4
  %2 = load float*, float** %Cr.addr, align 8
  store float 0.000000e+00, float* %2, align 4
  %3 = load float*, float** %Cg.addr, align 8
  store float 0.000000e+00, float* %3, align 4
  %4 = load float*, float** %Cb.addr, align 8
  store float 0.000000e+00, float* %4, align 4
  store float 0.000000e+00, float* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %5 = load float, float* %i, align 4
  %conv = fpext float %5 to double
  %cmp = fcmp olt double %conv, 4.000000e+00
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load float, float* %u1, align 4
  %conv1 = fpext float %6 to double
  %mul = fmul double %conv1, 1.500000e+00
  %conv2 = fptrunc double %mul to float
  store float %conv2, float* %t.addr.i43, align 4
  %7 = load float, float* %t.addr.i43, align 4
  %8 = load float, float* %t.addr.i43, align 4
  store float %8, float* %t.addr.i76, align 4
  %9 = load float, float* %t.addr.i76, align 4
  %cmp.i77 = fcmp oge float %9, 0.000000e+00
  br i1 %cmp.i77, label %if.then.i81, label %if.else.i

if.then.i81:                                      ; preds = %for.body
  %10 = load float, float* %t.addr.i76, align 4
  %conv.i78 = fptosi float %10 to i32
  %conv1.i79 = sitofp i32 %conv.i78 to double
  %conv2.i80 = fptrunc double %conv1.i79 to float
  store float %conv2.i80, float* %retval.i75, align 4
  br label %"?floor@@YAMM@Z.exit"

if.else.i:                                        ; preds = %for.body
  %11 = load float, float* %t.addr.i76, align 4
  %12 = load float, float* %t.addr.i76, align 4
  %conv3.i82 = fptosi float %12 to i32
  %conv4.i = sitofp i32 %conv3.i82 to float
  %cmp5.i = fcmp oeq float %11, %conv4.i
  br i1 %cmp5.i, label %if.then6.i, label %if.end.i85

if.then6.i:                                       ; preds = %if.else.i
  %13 = load float, float* %t.addr.i76, align 4
  store float %13, float* %retval.i75, align 4
  br label %"?floor@@YAMM@Z.exit"

if.end.i85:                                       ; preds = %if.else.i
  %14 = load float, float* %t.addr.i76, align 4
  %conv7.i83 = fptosi float %14 to i32
  %sub.i84 = sub nsw i32 %conv7.i83, 1
  %conv8.i = sitofp i32 %sub.i84 to double
  %conv9.i = fptrunc double %conv8.i to float
  store float %conv9.i, float* %retval.i75, align 4
  br label %"?floor@@YAMM@Z.exit"

"?floor@@YAMM@Z.exit":                            ; preds = %if.then.i81, %if.then6.i, %if.end.i85
  %15 = load float, float* %retval.i75, align 4
  %sub.i45 = fsub float %7, %15
  %conv3 = fpext float %sub.i45 to double
  %sub = fsub double %conv3, 5.000000e-01
  %conv4 = fptrunc double %sub to float
  store float %conv4, float* %u1, align 4
  %16 = load float, float* %v1, align 4
  %conv5 = fpext float %16 to double
  %mul6 = fmul double %conv5, 1.500000e+00
  %conv7 = fptrunc double %mul6 to float
  store float %conv7, float* %t.addr.i, align 4
  %17 = load float, float* %t.addr.i, align 4
  %18 = load float, float* %t.addr.i, align 4
  store float %18, float* %t.addr.i87, align 4
  %19 = load float, float* %t.addr.i87, align 4
  %cmp.i88 = fcmp oge float %19, 0.000000e+00
  br i1 %cmp.i88, label %if.then.i92, label %if.else.i96

if.then.i92:                                      ; preds = %"?floor@@YAMM@Z.exit"
  %20 = load float, float* %t.addr.i87, align 4
  %conv.i89 = fptosi float %20 to i32
  %conv1.i90 = sitofp i32 %conv.i89 to double
  %conv2.i91 = fptrunc double %conv1.i90 to float
  store float %conv2.i91, float* %retval.i86, align 4
  br label %"?floor@@YAMM@Z.exit103"

if.else.i96:                                      ; preds = %"?floor@@YAMM@Z.exit"
  %21 = load float, float* %t.addr.i87, align 4
  %22 = load float, float* %t.addr.i87, align 4
  %conv3.i93 = fptosi float %22 to i32
  %conv4.i94 = sitofp i32 %conv3.i93 to float
  %cmp5.i95 = fcmp oeq float %21, %conv4.i94
  br i1 %cmp5.i95, label %if.then6.i97, label %if.end.i102

if.then6.i97:                                     ; preds = %if.else.i96
  %23 = load float, float* %t.addr.i87, align 4
  store float %23, float* %retval.i86, align 4
  br label %"?floor@@YAMM@Z.exit103"

if.end.i102:                                      ; preds = %if.else.i96
  %24 = load float, float* %t.addr.i87, align 4
  %conv7.i98 = fptosi float %24 to i32
  %sub.i99 = sub nsw i32 %conv7.i98, 1
  %conv8.i100 = sitofp i32 %sub.i99 to double
  %conv9.i101 = fptrunc double %conv8.i100 to float
  store float %conv9.i101, float* %retval.i86, align 4
  br label %"?floor@@YAMM@Z.exit103"

"?floor@@YAMM@Z.exit103":                         ; preds = %if.then.i92, %if.then6.i97, %if.end.i102
  %25 = load float, float* %retval.i86, align 4
  %sub.i = fsub float %17, %25
  %conv9 = fpext float %sub.i to double
  %sub10 = fsub double %conv9, 5.000000e-01
  %conv11 = fptrunc double %sub10 to float
  store float %conv11, float* %v1, align 4
  %26 = load float, float* %v1, align 4
  %27 = load float, float* %u1, align 4
  store float %26, float* %b.addr.i53, align 4
  store float %27, float* %a.addr.i54, align 4
  %28 = load float, float* %a.addr.i54, align 4
  %29 = load float, float* %a.addr.i54, align 4
  %mul.i55 = fmul float %28, %29
  %30 = load float, float* %b.addr.i53, align 4
  %31 = load float, float* %b.addr.i53, align 4
  %mul1.i56 = fmul float %30, %31
  %add.i57 = fadd float %mul.i55, %mul1.i56
  store float %add.i57, float* %value.addr.i, align 4
  %32 = load float, float* %value.addr.i, align 4
  %cmp.i105 = fcmp olt float %32, 0.000000e+00
  br i1 %cmp.i105, label %if.then.i106, label %if.end.i107

if.then.i106:                                     ; preds = %"?floor@@YAMM@Z.exit103"
  store float -1.000000e+00, float* %retval.i104, align 4
  br label %"?sqrt@@YAMM@Z.exit"

if.end.i107:                                      ; preds = %"?floor@@YAMM@Z.exit103"
  %33 = load float, float* %value.addr.i, align 4
  %cmp1.i = fcmp oeq float %33, 0.000000e+00
  br i1 %cmp1.i, label %if.then2.i, label %if.end3.i

if.then2.i:                                       ; preds = %if.end.i107
  store float 0.000000e+00, float* %retval.i104, align 4
  br label %"?sqrt@@YAMM@Z.exit"

if.end3.i:                                        ; preds = %if.end.i107
  %34 = load float, float* %value.addr.i, align 4
  store float %34, float* %approx.i, align 4
  store float 0.000000e+00, float* %prev_approx.i, align 4
  br label %while.cond.i

while.cond.i:                                     ; preds = %while.body.i, %if.end3.i
  %35 = load float, float* %approx.i, align 4
  %36 = load float, float* %prev_approx.i, align 4
  %sub.i108 = fsub float %35, %36
  %conv.i109 = fpext float %sub.i108 to double
  %cmp4.i = fcmp ogt double %conv.i109, 0x3EB0C6F7A0B5ED8D
  br i1 %cmp4.i, label %lor.end.i, label %lor.rhs.i

lor.rhs.i:                                        ; preds = %while.cond.i
  %37 = load float, float* %prev_approx.i, align 4
  %38 = load float, float* %approx.i, align 4
  %sub5.i = fsub float %37, %38
  %conv6.i110 = fpext float %sub5.i to double
  %cmp7.i = fcmp ogt double %conv6.i110, 0x3EB0C6F7A0B5ED8D
  br label %lor.end.i

lor.end.i:                                        ; preds = %lor.rhs.i, %while.cond.i
  %39 = phi i1 [ true, %while.cond.i ], [ %cmp7.i, %lor.rhs.i ]
  br i1 %39, label %while.body.i, label %while.end.i

while.body.i:                                     ; preds = %lor.end.i
  %40 = load float, float* %approx.i, align 4
  store float %40, float* %prev_approx.i, align 4
  %41 = load float, float* %approx.i, align 4
  %42 = load float, float* %value.addr.i, align 4
  %43 = load float, float* %approx.i, align 4
  %div.i111 = fdiv float %42, %43
  %add.i112 = fadd float %41, %div.i111
  %conv8.i113 = fpext float %add.i112 to double
  %mul.i114 = fmul double 5.000000e-01, %conv8.i113
  %conv9.i115 = fptrunc double %mul.i114 to float
  store float %conv9.i115, float* %approx.i, align 4
  br label %while.cond.i, !llvm.loop !4

while.end.i:                                      ; preds = %lor.end.i
  %44 = load float, float* %approx.i, align 4
  store float %44, float* %retval.i104, align 4
  br label %"?sqrt@@YAMM@Z.exit"

"?sqrt@@YAMM@Z.exit":                             ; preds = %if.then.i106, %if.then2.i, %while.end.i
  %45 = load float, float* %retval.i104, align 4
  %46 = load float, float* %v.addr, align 4
  %47 = load float, float* %u.addr, align 4
  store float %46, float* %b.addr.i47, align 4
  store float %47, float* %a.addr.i48, align 4
  %48 = load float, float* %a.addr.i48, align 4
  %49 = load float, float* %a.addr.i48, align 4
  %mul.i49 = fmul float %48, %49
  %50 = load float, float* %b.addr.i47, align 4
  %51 = load float, float* %b.addr.i47, align 4
  %mul1.i50 = fmul float %50, %51
  %add.i51 = fadd float %mul.i49, %mul1.i50
  store float %add.i51, float* %value.addr.i117, align 4
  %52 = load float, float* %value.addr.i117, align 4
  %cmp.i120 = fcmp olt float %52, 0.000000e+00
  br i1 %cmp.i120, label %if.then.i121, label %if.end.i123

if.then.i121:                                     ; preds = %"?sqrt@@YAMM@Z.exit"
  store float -1.000000e+00, float* %retval.i116, align 4
  br label %"?sqrt@@YAMM@Z.exit142"

if.end.i123:                                      ; preds = %"?sqrt@@YAMM@Z.exit"
  %53 = load float, float* %value.addr.i117, align 4
  %cmp1.i122 = fcmp oeq float %53, 0.000000e+00
  br i1 %cmp1.i122, label %if.then2.i124, label %if.end3.i125

if.then2.i124:                                    ; preds = %if.end.i123
  store float 0.000000e+00, float* %retval.i116, align 4
  br label %"?sqrt@@YAMM@Z.exit142"

if.end3.i125:                                     ; preds = %if.end.i123
  %54 = load float, float* %value.addr.i117, align 4
  store float %54, float* %approx.i118, align 4
  store float 0.000000e+00, float* %prev_approx.i119, align 4
  br label %while.cond.i129

while.cond.i129:                                  ; preds = %while.body.i140, %if.end3.i125
  %55 = load float, float* %approx.i118, align 4
  %56 = load float, float* %prev_approx.i119, align 4
  %sub.i126 = fsub float %55, %56
  %conv.i127 = fpext float %sub.i126 to double
  %cmp4.i128 = fcmp ogt double %conv.i127, 0x3EB0C6F7A0B5ED8D
  br i1 %cmp4.i128, label %lor.end.i134, label %lor.rhs.i133

lor.rhs.i133:                                     ; preds = %while.cond.i129
  %57 = load float, float* %prev_approx.i119, align 4
  %58 = load float, float* %approx.i118, align 4
  %sub5.i130 = fsub float %57, %58
  %conv6.i131 = fpext float %sub5.i130 to double
  %cmp7.i132 = fcmp ogt double %conv6.i131, 0x3EB0C6F7A0B5ED8D
  br label %lor.end.i134

lor.end.i134:                                     ; preds = %lor.rhs.i133, %while.cond.i129
  %59 = phi i1 [ true, %while.cond.i129 ], [ %cmp7.i132, %lor.rhs.i133 ]
  br i1 %59, label %while.body.i140, label %while.end.i141

while.body.i140:                                  ; preds = %lor.end.i134
  %60 = load float, float* %approx.i118, align 4
  store float %60, float* %prev_approx.i119, align 4
  %61 = load float, float* %approx.i118, align 4
  %62 = load float, float* %value.addr.i117, align 4
  %63 = load float, float* %approx.i118, align 4
  %div.i135 = fdiv float %62, %63
  %add.i136 = fadd float %61, %div.i135
  %conv8.i137 = fpext float %add.i136 to double
  %mul.i138 = fmul double 5.000000e-01, %conv8.i137
  %conv9.i139 = fptrunc double %mul.i138 to float
  store float %conv9.i139, float* %approx.i118, align 4
  br label %while.cond.i129, !llvm.loop !4

while.end.i141:                                   ; preds = %lor.end.i134
  %64 = load float, float* %approx.i118, align 4
  store float %64, float* %retval.i116, align 4
  br label %"?sqrt@@YAMM@Z.exit142"

"?sqrt@@YAMM@Z.exit142":                          ; preds = %if.then.i121, %if.then2.i124, %while.end.i141
  %65 = load float, float* %retval.i116, align 4
  %fneg = fneg float %65
  store float %fneg, float* %x.addr.i, align 4
  store i32 100, i32* %N.i, align 4
  store float 1.000000e+00, float* %sum.i, align 4
  store float 1.000000e+00, float* %term.i, align 4
  store i32 1, i32* %i.i, align 4
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.body.i, %"?sqrt@@YAMM@Z.exit142"
  %66 = load i32, i32* %i.i, align 4
  %cmp.i = icmp slt i32 %66, 100
  br i1 %cmp.i, label %for.body.i, label %"?exp@@YAMM@Z.exit"

for.body.i:                                       ; preds = %for.cond.i
  %67 = load float, float* %term.i, align 4
  %68 = load float, float* %x.addr.i, align 4
  %mul.i59 = fmul float %67, %68
  %69 = load i32, i32* %i.i, align 4
  %conv.i = sitofp i32 %69 to float
  %div.i = fdiv float %mul.i59, %conv.i
  store float %div.i, float* %term.i, align 4
  %70 = load float, float* %term.i, align 4
  %71 = load float, float* %sum.i, align 4
  %add.i60 = fadd float %71, %70
  store float %add.i60, float* %sum.i, align 4
  %72 = load i32, i32* %i.i, align 4
  %inc.i = add nsw i32 %72, 1
  store i32 %inc.i, i32* %i.i, align 4
  br label %for.cond.i, !llvm.loop !6

"?exp@@YAMM@Z.exit":                              ; preds = %for.cond.i
  %73 = load float, float* %sum.i, align 4
  %mul15 = fmul float %45, %73
  store float %mul15, float* %d, align 4
  %74 = load float, float* %v.addr, align 4
  %75 = load float, float* %u.addr, align 4
  store float %74, float* %b.addr.i, align 4
  store float %75, float* %a.addr.i, align 4
  %76 = load float, float* %a.addr.i, align 4
  %77 = load float, float* %a.addr.i, align 4
  %mul.i = fmul float %76, %77
  %78 = load float, float* %b.addr.i, align 4
  %79 = load float, float* %b.addr.i, align 4
  %mul1.i = fmul float %78, %79
  %add.i = fadd float %mul.i, %mul1.i
  store float %add.i, float* %value.addr.i144, align 4
  %80 = load float, float* %value.addr.i144, align 4
  %cmp.i147 = fcmp olt float %80, 0.000000e+00
  br i1 %cmp.i147, label %if.then.i148, label %if.end.i150

if.then.i148:                                     ; preds = %"?exp@@YAMM@Z.exit"
  store float -1.000000e+00, float* %retval.i143, align 4
  br label %"?sqrt@@YAMM@Z.exit169"

if.end.i150:                                      ; preds = %"?exp@@YAMM@Z.exit"
  %81 = load float, float* %value.addr.i144, align 4
  %cmp1.i149 = fcmp oeq float %81, 0.000000e+00
  br i1 %cmp1.i149, label %if.then2.i151, label %if.end3.i152

if.then2.i151:                                    ; preds = %if.end.i150
  store float 0.000000e+00, float* %retval.i143, align 4
  br label %"?sqrt@@YAMM@Z.exit169"

if.end3.i152:                                     ; preds = %if.end.i150
  %82 = load float, float* %value.addr.i144, align 4
  store float %82, float* %approx.i145, align 4
  store float 0.000000e+00, float* %prev_approx.i146, align 4
  br label %while.cond.i156

while.cond.i156:                                  ; preds = %while.body.i167, %if.end3.i152
  %83 = load float, float* %approx.i145, align 4
  %84 = load float, float* %prev_approx.i146, align 4
  %sub.i153 = fsub float %83, %84
  %conv.i154 = fpext float %sub.i153 to double
  %cmp4.i155 = fcmp ogt double %conv.i154, 0x3EB0C6F7A0B5ED8D
  br i1 %cmp4.i155, label %lor.end.i161, label %lor.rhs.i160

lor.rhs.i160:                                     ; preds = %while.cond.i156
  %85 = load float, float* %prev_approx.i146, align 4
  %86 = load float, float* %approx.i145, align 4
  %sub5.i157 = fsub float %85, %86
  %conv6.i158 = fpext float %sub5.i157 to double
  %cmp7.i159 = fcmp ogt double %conv6.i158, 0x3EB0C6F7A0B5ED8D
  br label %lor.end.i161

lor.end.i161:                                     ; preds = %lor.rhs.i160, %while.cond.i156
  %87 = phi i1 [ true, %while.cond.i156 ], [ %cmp7.i159, %lor.rhs.i160 ]
  br i1 %87, label %while.body.i167, label %while.end.i168

while.body.i167:                                  ; preds = %lor.end.i161
  %88 = load float, float* %approx.i145, align 4
  store float %88, float* %prev_approx.i146, align 4
  %89 = load float, float* %approx.i145, align 4
  %90 = load float, float* %value.addr.i144, align 4
  %91 = load float, float* %approx.i145, align 4
  %div.i162 = fdiv float %90, %91
  %add.i163 = fadd float %89, %div.i162
  %conv8.i164 = fpext float %add.i163 to double
  %mul.i165 = fmul double 5.000000e-01, %conv8.i164
  %conv9.i166 = fptrunc double %mul.i165 to float
  store float %conv9.i166, float* %approx.i145, align 4
  br label %while.cond.i156, !llvm.loop !4

while.end.i168:                                   ; preds = %lor.end.i161
  %92 = load float, float* %approx.i145, align 4
  store float %92, float* %retval.i143, align 4
  br label %"?sqrt@@YAMM@Z.exit169"

"?sqrt@@YAMM@Z.exit169":                          ; preds = %if.then.i148, %if.then2.i151, %while.end.i168
  %93 = load float, float* %retval.i143, align 4
  %conv17 = fpext float %93 to double
  %94 = load float, float* %i, align 4
  %conv18 = fpext float %94 to double
  %mul19 = fmul double %conv18, 4.000000e-01
  %add = fadd double %conv17, %mul19
  %95 = load float, float* %t, align 4
  %conv20 = fpext float %95 to double
  %mul21 = fmul double %conv20, 4.000000e-01
  %add22 = fadd double %add, %mul21
  %conv23 = fptrunc double %add22 to float
  store float* %colz, float** %oz.addr.i, align 8
  store float* %coly, float** %oy.addr.i, align 8
  store float* %colx, float** %ox.addr.i, align 8
  store float %conv23, float* %t.addr.i61, align 4
  %96 = load float, float* %t.addr.i61, align 4
  %conv.i62 = fpext float %96 to double
  %add.i63 = fadd double %conv.i62, 2.630000e-01
  %mul1.i64 = fmul double 6.283180e+00, %add.i63
  %conv2.i = fptrunc double %mul1.i64 to float
  store float %conv2.i, float* %x.addr.i271, align 4
  store float 0x3FC45F3060000000, float* %tp.i272, align 4
  %97 = load float, float* %tp.i272, align 4
  %98 = load float, float* %x.addr.i271, align 4
  %mul.i273 = fmul float %98, %97
  store float %mul.i273, float* %x.addr.i271, align 4
  %99 = load float, float* %x.addr.i271, align 4
  %add.i274 = fadd float %99, 2.500000e-01
  store float %add.i274, float* %t.addr.i18.i268, align 4
  %100 = load float, float* %t.addr.i18.i268, align 4
  %cmp.i19.i275 = fcmp oge float %100, 0.000000e+00
  br i1 %cmp.i19.i275, label %if.then.i.i279, label %if.else.i.i283

if.then.i.i279:                                   ; preds = %"?sqrt@@YAMM@Z.exit169"
  %101 = load float, float* %t.addr.i18.i268, align 4
  %conv.i.i276 = fptosi float %101 to i32
  %conv1.i.i277 = sitofp i32 %conv.i.i276 to double
  %conv2.i.i278 = fptrunc double %conv1.i.i277 to float
  store float %conv2.i.i278, float* %retval.i.i267, align 4
  br label %"?floor@@YAMM@Z.exit.i293"

if.else.i.i283:                                   ; preds = %"?sqrt@@YAMM@Z.exit169"
  %102 = load float, float* %t.addr.i18.i268, align 4
  %103 = load float, float* %t.addr.i18.i268, align 4
  %conv3.i.i280 = fptosi float %103 to i32
  %conv4.i.i281 = sitofp i32 %conv3.i.i280 to float
  %cmp5.i.i282 = fcmp oeq float %102, %conv4.i.i281
  br i1 %cmp5.i.i282, label %if.then6.i.i284, label %if.end.i.i289

if.then6.i.i284:                                  ; preds = %if.else.i.i283
  %104 = load float, float* %t.addr.i18.i268, align 4
  store float %104, float* %retval.i.i267, align 4
  br label %"?floor@@YAMM@Z.exit.i293"

if.end.i.i289:                                    ; preds = %if.else.i.i283
  %105 = load float, float* %t.addr.i18.i268, align 4
  %conv7.i.i285 = fptosi float %105 to i32
  %sub.i.i286 = sub nsw i32 %conv7.i.i285, 1
  %conv8.i.i287 = sitofp i32 %sub.i.i286 to double
  %conv9.i.i288 = fptrunc double %conv8.i.i287 to float
  store float %conv9.i.i288, float* %retval.i.i267, align 4
  br label %"?floor@@YAMM@Z.exit.i293"

"?floor@@YAMM@Z.exit.i293":                       ; preds = %if.end.i.i289, %if.then6.i.i284, %if.then.i.i279
  %106 = load float, float* %retval.i.i267, align 4
  %add1.i290 = fadd float 2.500000e-01, %106
  %107 = load float, float* %x.addr.i271, align 4
  %sub.i291 = fsub float %107, %add1.i290
  store float %sub.i291, float* %x.addr.i271, align 4
  %108 = load float, float* %x.addr.i271, align 4
  store float %108, float* %t.addr.i11.i269, align 4
  %109 = load float, float* %t.addr.i11.i269, align 4
  %cmp.i12.i292 = fcmp oge float %109, 0.000000e+00
  br i1 %cmp.i12.i292, label %cond.true.i13.i294, label %cond.false.i15.i296

cond.true.i13.i294:                               ; preds = %"?floor@@YAMM@Z.exit.i293"
  %110 = load float, float* %t.addr.i11.i269, align 4
  br label %"?abs@@YAMM@Z.exit17.i303"

cond.false.i15.i296:                              ; preds = %"?floor@@YAMM@Z.exit.i293"
  %111 = load float, float* %t.addr.i11.i269, align 4
  %fneg.i14.i295 = fneg float %111
  br label %"?abs@@YAMM@Z.exit17.i303"

"?abs@@YAMM@Z.exit17.i303":                       ; preds = %cond.false.i15.i296, %cond.true.i13.i294
  %cond.i16.i297 = phi float [ %110, %cond.true.i13.i294 ], [ %fneg.i14.i295, %cond.false.i15.i296 ]
  %sub3.i298 = fsub float %cond.i16.i297, 5.000000e-01
  %mul4.i299 = fmul float 1.600000e+01, %sub3.i298
  %112 = load float, float* %x.addr.i271, align 4
  %mul5.i300 = fmul float %112, %mul4.i299
  store float %mul5.i300, float* %x.addr.i271, align 4
  %113 = load float, float* %x.addr.i271, align 4
  %mul6.i301 = fmul float 0x3FCCCCCCC0000000, %113
  %114 = load float, float* %x.addr.i271, align 4
  store float %114, float* %t.addr.i.i270, align 4
  %115 = load float, float* %t.addr.i.i270, align 4
  %cmp.i.i302 = fcmp oge float %115, 0.000000e+00
  br i1 %cmp.i.i302, label %cond.true.i.i304, label %cond.false.i.i306

cond.true.i.i304:                                 ; preds = %"?abs@@YAMM@Z.exit17.i303"
  %116 = load float, float* %t.addr.i.i270, align 4
  br label %"?cos@@YAMM@Z.exit311"

cond.false.i.i306:                                ; preds = %"?abs@@YAMM@Z.exit17.i303"
  %117 = load float, float* %t.addr.i.i270, align 4
  %fneg.i.i305 = fneg float %117
  br label %"?cos@@YAMM@Z.exit311"

"?cos@@YAMM@Z.exit311":                           ; preds = %cond.true.i.i304, %cond.false.i.i306
  %cond.i.i307 = phi float [ %116, %cond.true.i.i304 ], [ %fneg.i.i305, %cond.false.i.i306 ]
  %sub8.i308 = fsub float %cond.i.i307, 1.000000e+00
  %mul9.i309 = fmul float %mul6.i301, %sub8.i308
  %118 = load float, float* %x.addr.i271, align 4
  %add10.i310 = fadd float %118, %mul9.i309
  store float %add10.i310, float* %x.addr.i271, align 4
  %119 = load float, float* %x.addr.i271, align 4
  %conv3.i = fpext float %119 to double
  %mul4.i = fmul double 5.000000e-01, %conv3.i
  %add5.i = fadd double 5.000000e-01, %mul4.i
  %conv6.i = fptrunc double %add5.i to float
  %120 = load float*, float** %ox.addr.i, align 8
  store float %conv6.i, float* %120, align 4
  %121 = load float, float* %t.addr.i61, align 4
  %conv7.i = fpext float %121 to double
  %add9.i = fadd double %conv7.i, 4.160000e-01
  %mul10.i = fmul double 6.283180e+00, %add9.i
  %conv11.i = fptrunc double %mul10.i to float
  store float %conv11.i, float* %x.addr.i226, align 4
  store float 0x3FC45F3060000000, float* %tp.i227, align 4
  %122 = load float, float* %tp.i227, align 4
  %123 = load float, float* %x.addr.i226, align 4
  %mul.i228 = fmul float %123, %122
  store float %mul.i228, float* %x.addr.i226, align 4
  %124 = load float, float* %x.addr.i226, align 4
  %add.i229 = fadd float %124, 2.500000e-01
  store float %add.i229, float* %t.addr.i18.i223, align 4
  %125 = load float, float* %t.addr.i18.i223, align 4
  %cmp.i19.i230 = fcmp oge float %125, 0.000000e+00
  br i1 %cmp.i19.i230, label %if.then.i.i234, label %if.else.i.i238

if.then.i.i234:                                   ; preds = %"?cos@@YAMM@Z.exit311"
  %126 = load float, float* %t.addr.i18.i223, align 4
  %conv.i.i231 = fptosi float %126 to i32
  %conv1.i.i232 = sitofp i32 %conv.i.i231 to double
  %conv2.i.i233 = fptrunc double %conv1.i.i232 to float
  store float %conv2.i.i233, float* %retval.i.i222, align 4
  br label %"?floor@@YAMM@Z.exit.i248"

if.else.i.i238:                                   ; preds = %"?cos@@YAMM@Z.exit311"
  %127 = load float, float* %t.addr.i18.i223, align 4
  %128 = load float, float* %t.addr.i18.i223, align 4
  %conv3.i.i235 = fptosi float %128 to i32
  %conv4.i.i236 = sitofp i32 %conv3.i.i235 to float
  %cmp5.i.i237 = fcmp oeq float %127, %conv4.i.i236
  br i1 %cmp5.i.i237, label %if.then6.i.i239, label %if.end.i.i244

if.then6.i.i239:                                  ; preds = %if.else.i.i238
  %129 = load float, float* %t.addr.i18.i223, align 4
  store float %129, float* %retval.i.i222, align 4
  br label %"?floor@@YAMM@Z.exit.i248"

if.end.i.i244:                                    ; preds = %if.else.i.i238
  %130 = load float, float* %t.addr.i18.i223, align 4
  %conv7.i.i240 = fptosi float %130 to i32
  %sub.i.i241 = sub nsw i32 %conv7.i.i240, 1
  %conv8.i.i242 = sitofp i32 %sub.i.i241 to double
  %conv9.i.i243 = fptrunc double %conv8.i.i242 to float
  store float %conv9.i.i243, float* %retval.i.i222, align 4
  br label %"?floor@@YAMM@Z.exit.i248"

"?floor@@YAMM@Z.exit.i248":                       ; preds = %if.end.i.i244, %if.then6.i.i239, %if.then.i.i234
  %131 = load float, float* %retval.i.i222, align 4
  %add1.i245 = fadd float 2.500000e-01, %131
  %132 = load float, float* %x.addr.i226, align 4
  %sub.i246 = fsub float %132, %add1.i245
  store float %sub.i246, float* %x.addr.i226, align 4
  %133 = load float, float* %x.addr.i226, align 4
  store float %133, float* %t.addr.i11.i224, align 4
  %134 = load float, float* %t.addr.i11.i224, align 4
  %cmp.i12.i247 = fcmp oge float %134, 0.000000e+00
  br i1 %cmp.i12.i247, label %cond.true.i13.i249, label %cond.false.i15.i251

cond.true.i13.i249:                               ; preds = %"?floor@@YAMM@Z.exit.i248"
  %135 = load float, float* %t.addr.i11.i224, align 4
  br label %"?abs@@YAMM@Z.exit17.i258"

cond.false.i15.i251:                              ; preds = %"?floor@@YAMM@Z.exit.i248"
  %136 = load float, float* %t.addr.i11.i224, align 4
  %fneg.i14.i250 = fneg float %136
  br label %"?abs@@YAMM@Z.exit17.i258"

"?abs@@YAMM@Z.exit17.i258":                       ; preds = %cond.false.i15.i251, %cond.true.i13.i249
  %cond.i16.i252 = phi float [ %135, %cond.true.i13.i249 ], [ %fneg.i14.i250, %cond.false.i15.i251 ]
  %sub3.i253 = fsub float %cond.i16.i252, 5.000000e-01
  %mul4.i254 = fmul float 1.600000e+01, %sub3.i253
  %137 = load float, float* %x.addr.i226, align 4
  %mul5.i255 = fmul float %137, %mul4.i254
  store float %mul5.i255, float* %x.addr.i226, align 4
  %138 = load float, float* %x.addr.i226, align 4
  %mul6.i256 = fmul float 0x3FCCCCCCC0000000, %138
  %139 = load float, float* %x.addr.i226, align 4
  store float %139, float* %t.addr.i.i225, align 4
  %140 = load float, float* %t.addr.i.i225, align 4
  %cmp.i.i257 = fcmp oge float %140, 0.000000e+00
  br i1 %cmp.i.i257, label %cond.true.i.i259, label %cond.false.i.i261

cond.true.i.i259:                                 ; preds = %"?abs@@YAMM@Z.exit17.i258"
  %141 = load float, float* %t.addr.i.i225, align 4
  br label %"?cos@@YAMM@Z.exit266"

cond.false.i.i261:                                ; preds = %"?abs@@YAMM@Z.exit17.i258"
  %142 = load float, float* %t.addr.i.i225, align 4
  %fneg.i.i260 = fneg float %142
  br label %"?cos@@YAMM@Z.exit266"

"?cos@@YAMM@Z.exit266":                           ; preds = %cond.true.i.i259, %cond.false.i.i261
  %cond.i.i262 = phi float [ %141, %cond.true.i.i259 ], [ %fneg.i.i260, %cond.false.i.i261 ]
  %sub8.i263 = fsub float %cond.i.i262, 1.000000e+00
  %mul9.i264 = fmul float %mul6.i256, %sub8.i263
  %143 = load float, float* %x.addr.i226, align 4
  %add10.i265 = fadd float %143, %mul9.i264
  store float %add10.i265, float* %x.addr.i226, align 4
  %144 = load float, float* %x.addr.i226, align 4
  %conv13.i = fpext float %144 to double
  %mul14.i = fmul double 5.000000e-01, %conv13.i
  %add15.i = fadd double 5.000000e-01, %mul14.i
  %conv16.i = fptrunc double %add15.i to float
  %145 = load float*, float** %oy.addr.i, align 8
  store float %conv16.i, float* %145, align 4
  %146 = load float, float* %t.addr.i61, align 4
  %conv17.i = fpext float %146 to double
  %add19.i = fadd double %conv17.i, 5.570000e-01
  %mul20.i = fmul double 6.283180e+00, %add19.i
  %conv21.i = fptrunc double %mul20.i to float
  store float %conv21.i, float* %x.addr.i181, align 4
  store float 0x3FC45F3060000000, float* %tp.i182, align 4
  %147 = load float, float* %tp.i182, align 4
  %148 = load float, float* %x.addr.i181, align 4
  %mul.i183 = fmul float %148, %147
  store float %mul.i183, float* %x.addr.i181, align 4
  %149 = load float, float* %x.addr.i181, align 4
  %add.i184 = fadd float %149, 2.500000e-01
  store float %add.i184, float* %t.addr.i18.i178, align 4
  %150 = load float, float* %t.addr.i18.i178, align 4
  %cmp.i19.i185 = fcmp oge float %150, 0.000000e+00
  br i1 %cmp.i19.i185, label %if.then.i.i189, label %if.else.i.i193

if.then.i.i189:                                   ; preds = %"?cos@@YAMM@Z.exit266"
  %151 = load float, float* %t.addr.i18.i178, align 4
  %conv.i.i186 = fptosi float %151 to i32
  %conv1.i.i187 = sitofp i32 %conv.i.i186 to double
  %conv2.i.i188 = fptrunc double %conv1.i.i187 to float
  store float %conv2.i.i188, float* %retval.i.i177, align 4
  br label %"?floor@@YAMM@Z.exit.i203"

if.else.i.i193:                                   ; preds = %"?cos@@YAMM@Z.exit266"
  %152 = load float, float* %t.addr.i18.i178, align 4
  %153 = load float, float* %t.addr.i18.i178, align 4
  %conv3.i.i190 = fptosi float %153 to i32
  %conv4.i.i191 = sitofp i32 %conv3.i.i190 to float
  %cmp5.i.i192 = fcmp oeq float %152, %conv4.i.i191
  br i1 %cmp5.i.i192, label %if.then6.i.i194, label %if.end.i.i199

if.then6.i.i194:                                  ; preds = %if.else.i.i193
  %154 = load float, float* %t.addr.i18.i178, align 4
  store float %154, float* %retval.i.i177, align 4
  br label %"?floor@@YAMM@Z.exit.i203"

if.end.i.i199:                                    ; preds = %if.else.i.i193
  %155 = load float, float* %t.addr.i18.i178, align 4
  %conv7.i.i195 = fptosi float %155 to i32
  %sub.i.i196 = sub nsw i32 %conv7.i.i195, 1
  %conv8.i.i197 = sitofp i32 %sub.i.i196 to double
  %conv9.i.i198 = fptrunc double %conv8.i.i197 to float
  store float %conv9.i.i198, float* %retval.i.i177, align 4
  br label %"?floor@@YAMM@Z.exit.i203"

"?floor@@YAMM@Z.exit.i203":                       ; preds = %if.end.i.i199, %if.then6.i.i194, %if.then.i.i189
  %156 = load float, float* %retval.i.i177, align 4
  %add1.i200 = fadd float 2.500000e-01, %156
  %157 = load float, float* %x.addr.i181, align 4
  %sub.i201 = fsub float %157, %add1.i200
  store float %sub.i201, float* %x.addr.i181, align 4
  %158 = load float, float* %x.addr.i181, align 4
  store float %158, float* %t.addr.i11.i179, align 4
  %159 = load float, float* %t.addr.i11.i179, align 4
  %cmp.i12.i202 = fcmp oge float %159, 0.000000e+00
  br i1 %cmp.i12.i202, label %cond.true.i13.i204, label %cond.false.i15.i206

cond.true.i13.i204:                               ; preds = %"?floor@@YAMM@Z.exit.i203"
  %160 = load float, float* %t.addr.i11.i179, align 4
  br label %"?abs@@YAMM@Z.exit17.i213"

cond.false.i15.i206:                              ; preds = %"?floor@@YAMM@Z.exit.i203"
  %161 = load float, float* %t.addr.i11.i179, align 4
  %fneg.i14.i205 = fneg float %161
  br label %"?abs@@YAMM@Z.exit17.i213"

"?abs@@YAMM@Z.exit17.i213":                       ; preds = %cond.false.i15.i206, %cond.true.i13.i204
  %cond.i16.i207 = phi float [ %160, %cond.true.i13.i204 ], [ %fneg.i14.i205, %cond.false.i15.i206 ]
  %sub3.i208 = fsub float %cond.i16.i207, 5.000000e-01
  %mul4.i209 = fmul float 1.600000e+01, %sub3.i208
  %162 = load float, float* %x.addr.i181, align 4
  %mul5.i210 = fmul float %162, %mul4.i209
  store float %mul5.i210, float* %x.addr.i181, align 4
  %163 = load float, float* %x.addr.i181, align 4
  %mul6.i211 = fmul float 0x3FCCCCCCC0000000, %163
  %164 = load float, float* %x.addr.i181, align 4
  store float %164, float* %t.addr.i.i180, align 4
  %165 = load float, float* %t.addr.i.i180, align 4
  %cmp.i.i212 = fcmp oge float %165, 0.000000e+00
  br i1 %cmp.i.i212, label %cond.true.i.i214, label %cond.false.i.i216

cond.true.i.i214:                                 ; preds = %"?abs@@YAMM@Z.exit17.i213"
  %166 = load float, float* %t.addr.i.i180, align 4
  br label %"?cos@@YAMM@Z.exit221"

cond.false.i.i216:                                ; preds = %"?abs@@YAMM@Z.exit17.i213"
  %167 = load float, float* %t.addr.i.i180, align 4
  %fneg.i.i215 = fneg float %167
  br label %"?cos@@YAMM@Z.exit221"

"?cos@@YAMM@Z.exit221":                           ; preds = %cond.true.i.i214, %cond.false.i.i216
  %cond.i.i217 = phi float [ %166, %cond.true.i.i214 ], [ %fneg.i.i215, %cond.false.i.i216 ]
  %sub8.i218 = fsub float %cond.i.i217, 1.000000e+00
  %mul9.i219 = fmul float %mul6.i211, %sub8.i218
  %168 = load float, float* %x.addr.i181, align 4
  %add10.i220 = fadd float %168, %mul9.i219
  store float %add10.i220, float* %x.addr.i181, align 4
  %169 = load float, float* %x.addr.i181, align 4
  %conv23.i = fpext float %169 to double
  %mul24.i = fmul double 5.000000e-01, %conv23.i
  %add25.i = fadd double 5.000000e-01, %mul24.i
  %conv26.i = fptrunc double %add25.i to float
  %170 = load float*, float** %oz.addr.i, align 8
  store float %conv26.i, float* %170, align 4
  %171 = load float, float* %d, align 4
  %conv24 = fpext float %171 to double
  %mul25 = fmul double %conv24, 8.000000e+00
  %172 = load float, float* %t, align 4
  %conv26 = fpext float %172 to double
  %add27 = fadd double %mul25, %conv26
  %conv28 = fptrunc double %add27 to float
  store float %conv28, float* %x.addr.i66, align 4
  %173 = load float, float* %x.addr.i66, align 4
  %conv.i67 = fpext float %173 to double
  %sub.i68 = fsub double %conv.i67, 0x3FF921FB54442D18
  %conv1.i = fptrunc double %sub.i68 to float
  store float %conv1.i, float* %x.addr.i170, align 4
  store float 0x3FC45F3060000000, float* %tp.i, align 4
  %174 = load float, float* %tp.i, align 4
  %175 = load float, float* %x.addr.i170, align 4
  %mul.i171 = fmul float %175, %174
  store float %mul.i171, float* %x.addr.i170, align 4
  %176 = load float, float* %x.addr.i170, align 4
  %add.i172 = fadd float %176, 2.500000e-01
  store float %add.i172, float* %t.addr.i18.i, align 4
  %177 = load float, float* %t.addr.i18.i, align 4
  %cmp.i19.i = fcmp oge float %177, 0.000000e+00
  br i1 %cmp.i19.i, label %if.then.i.i, label %if.else.i.i

if.then.i.i:                                      ; preds = %"?cos@@YAMM@Z.exit221"
  %178 = load float, float* %t.addr.i18.i, align 4
  %conv.i.i173 = fptosi float %178 to i32
  %conv1.i.i = sitofp i32 %conv.i.i173 to double
  %conv2.i.i = fptrunc double %conv1.i.i to float
  store float %conv2.i.i, float* %retval.i.i, align 4
  br label %"?floor@@YAMM@Z.exit.i"

if.else.i.i:                                      ; preds = %"?cos@@YAMM@Z.exit221"
  %179 = load float, float* %t.addr.i18.i, align 4
  %180 = load float, float* %t.addr.i18.i, align 4
  %conv3.i.i = fptosi float %180 to i32
  %conv4.i.i = sitofp i32 %conv3.i.i to float
  %cmp5.i.i = fcmp oeq float %179, %conv4.i.i
  br i1 %cmp5.i.i, label %if.then6.i.i, label %if.end.i.i

if.then6.i.i:                                     ; preds = %if.else.i.i
  %181 = load float, float* %t.addr.i18.i, align 4
  store float %181, float* %retval.i.i, align 4
  br label %"?floor@@YAMM@Z.exit.i"

if.end.i.i:                                       ; preds = %if.else.i.i
  %182 = load float, float* %t.addr.i18.i, align 4
  %conv7.i.i = fptosi float %182 to i32
  %sub.i.i = sub nsw i32 %conv7.i.i, 1
  %conv8.i.i = sitofp i32 %sub.i.i to double
  %conv9.i.i = fptrunc double %conv8.i.i to float
  store float %conv9.i.i, float* %retval.i.i, align 4
  br label %"?floor@@YAMM@Z.exit.i"

"?floor@@YAMM@Z.exit.i":                          ; preds = %if.end.i.i, %if.then6.i.i, %if.then.i.i
  %183 = load float, float* %retval.i.i, align 4
  %add1.i = fadd float 2.500000e-01, %183
  %184 = load float, float* %x.addr.i170, align 4
  %sub.i174 = fsub float %184, %add1.i
  store float %sub.i174, float* %x.addr.i170, align 4
  %185 = load float, float* %x.addr.i170, align 4
  store float %185, float* %t.addr.i11.i, align 4
  %186 = load float, float* %t.addr.i11.i, align 4
  %cmp.i12.i = fcmp oge float %186, 0.000000e+00
  br i1 %cmp.i12.i, label %cond.true.i13.i, label %cond.false.i15.i

cond.true.i13.i:                                  ; preds = %"?floor@@YAMM@Z.exit.i"
  %187 = load float, float* %t.addr.i11.i, align 4
  br label %"?abs@@YAMM@Z.exit17.i"

cond.false.i15.i:                                 ; preds = %"?floor@@YAMM@Z.exit.i"
  %188 = load float, float* %t.addr.i11.i, align 4
  %fneg.i14.i = fneg float %188
  br label %"?abs@@YAMM@Z.exit17.i"

"?abs@@YAMM@Z.exit17.i":                          ; preds = %cond.false.i15.i, %cond.true.i13.i
  %cond.i16.i = phi float [ %187, %cond.true.i13.i ], [ %fneg.i14.i, %cond.false.i15.i ]
  %sub3.i = fsub float %cond.i16.i, 5.000000e-01
  %mul4.i175 = fmul float 1.600000e+01, %sub3.i
  %189 = load float, float* %x.addr.i170, align 4
  %mul5.i = fmul float %189, %mul4.i175
  store float %mul5.i, float* %x.addr.i170, align 4
  %190 = load float, float* %x.addr.i170, align 4
  %mul6.i = fmul float 0x3FCCCCCCC0000000, %190
  %191 = load float, float* %x.addr.i170, align 4
  store float %191, float* %t.addr.i.i, align 4
  %192 = load float, float* %t.addr.i.i, align 4
  %cmp.i.i176 = fcmp oge float %192, 0.000000e+00
  br i1 %cmp.i.i176, label %cond.true.i.i, label %cond.false.i.i

cond.true.i.i:                                    ; preds = %"?abs@@YAMM@Z.exit17.i"
  %193 = load float, float* %t.addr.i.i, align 4
  br label %"?cos@@YAMM@Z.exit"

cond.false.i.i:                                   ; preds = %"?abs@@YAMM@Z.exit17.i"
  %194 = load float, float* %t.addr.i.i, align 4
  %fneg.i.i = fneg float %194
  br label %"?cos@@YAMM@Z.exit"

"?cos@@YAMM@Z.exit":                              ; preds = %cond.true.i.i, %cond.false.i.i
  %cond.i.i = phi float [ %193, %cond.true.i.i ], [ %fneg.i.i, %cond.false.i.i ]
  %sub8.i = fsub float %cond.i.i, 1.000000e+00
  %mul9.i = fmul float %mul6.i, %sub8.i
  %195 = load float, float* %x.addr.i170, align 4
  %add10.i = fadd float %195, %mul9.i
  store float %add10.i, float* %x.addr.i170, align 4
  %196 = load float, float* %x.addr.i170, align 4
  %conv30 = fpext float %196 to double
  %div = fdiv double %conv30, 8.000000e+00
  %conv31 = fptrunc double %div to float
  store float %conv31, float* %d, align 4
  %197 = load float, float* %d, align 4
  store float %197, float* %t.addr.i70, align 4
  %198 = load float, float* %t.addr.i70, align 4
  %cmp.i71 = fcmp oge float %198, 0.000000e+00
  br i1 %cmp.i71, label %cond.true.i, label %cond.false.i

cond.true.i:                                      ; preds = %"?cos@@YAMM@Z.exit"
  %199 = load float, float* %t.addr.i70, align 4
  br label %"?abs@@YAMM@Z.exit"

cond.false.i:                                     ; preds = %"?cos@@YAMM@Z.exit"
  %200 = load float, float* %t.addr.i70, align 4
  %fneg.i = fneg float %200
  br label %"?abs@@YAMM@Z.exit"

"?abs@@YAMM@Z.exit":                              ; preds = %cond.true.i, %cond.false.i
  %cond.i = phi float [ %199, %cond.true.i ], [ %fneg.i, %cond.false.i ]
  store float %cond.i, float* %d, align 4
  %201 = load float, float* %d, align 4
  %conv33 = fpext float %201 to double
  %div34 = fdiv double 1.000000e-01, %conv33
  %conv35 = fptrunc double %div34 to float
  store float 0x3FF3333340000000, float* %exponent.addr.i, align 4
  store float %conv35, float* %base.addr.i, align 4
  %202 = load float, float* %base.addr.i, align 4
  %cmp.i72 = fcmp oeq float %202, 0.000000e+00
  br i1 %cmp.i72, label %if.then.i, label %if.end.i

if.then.i:                                        ; preds = %"?abs@@YAMM@Z.exit"
  store float 0.000000e+00, float* %retval.i, align 4
  br label %"?pow@@YAMMM@Z.exit"

if.end.i:                                         ; preds = %"?abs@@YAMM@Z.exit"
  %203 = load float, float* %exponent.addr.i, align 4
  %204 = load float, float* %base.addr.i, align 4
  store float %204, float* %x.addr.i313, align 4
  %205 = load float, float* %x.addr.i313, align 4
  %cmp.i316 = fcmp ole float %205, 0.000000e+00
  br i1 %cmp.i316, label %if.then.i317, label %if.end.i322

if.then.i317:                                     ; preds = %if.end.i
  store float 0.000000e+00, float* %retval.i312, align 4
  br label %"?log@@YAMM@Z.exit"

if.end.i322:                                      ; preds = %if.end.i
  store float 0.000000e+00, float* %sum.i314, align 4
  %206 = load float, float* %x.addr.i313, align 4
  %sub.i318 = fsub float %206, 1.000000e+00
  %207 = load float, float* %x.addr.i313, align 4
  %add.i319 = fadd float %207, 1.000000e+00
  %div.i320 = fdiv float %sub.i318, %add.i319
  store float %div.i320, float* %term.i315, align 4
  %208 = load float, float* %term.i315, align 4
  %209 = load float, float* %term.i315, align 4
  %mul.i321 = fmul float %208, %209
  store float %mul.i321, float* %term_squared.i, align 4
  %210 = load float, float* %term.i315, align 4
  store float %210, float* %numerator.i, align 4
  store i32 1, i32* %n.i, align 4
  br label %for.cond.i324

for.cond.i324:                                    ; preds = %for.body.i327, %if.end.i322
  %211 = load i32, i32* %n.i, align 4
  %cmp1.i323 = icmp sle i32 %211, 100
  br i1 %cmp1.i323, label %for.body.i327, label %for.end.i

for.body.i327:                                    ; preds = %for.cond.i324
  %212 = load float, float* %numerator.i, align 4
  %213 = load i32, i32* %n.i, align 4
  %conv.i325 = sitofp i32 %213 to float
  %div2.i = fdiv float %212, %conv.i325
  %214 = load float, float* %sum.i314, align 4
  %add3.i = fadd float %214, %div2.i
  store float %add3.i, float* %sum.i314, align 4
  %215 = load float, float* %term_squared.i, align 4
  %216 = load float, float* %numerator.i, align 4
  %mul4.i326 = fmul float %216, %215
  store float %mul4.i326, float* %numerator.i, align 4
  %217 = load i32, i32* %n.i, align 4
  %add5.i328 = add nsw i32 %217, 2
  store i32 %add5.i328, i32* %n.i, align 4
  br label %for.cond.i324, !llvm.loop !7

for.end.i:                                        ; preds = %for.cond.i324
  %218 = load float, float* %sum.i314, align 4
  %mul6.i329 = fmul float 2.000000e+00, %218
  store float %mul6.i329, float* %retval.i312, align 4
  br label %"?log@@YAMM@Z.exit"

"?log@@YAMM@Z.exit":                              ; preds = %if.then.i317, %for.end.i
  %219 = load float, float* %retval.i312, align 4
  %mul.i74 = fmul float %203, %219
  store float %mul.i74, float* %x.addr.i.i, align 4
  store i32 100, i32* %N.i.i, align 4
  store float 1.000000e+00, float* %sum.i.i, align 4
  store float 1.000000e+00, float* %term.i.i, align 4
  store i32 1, i32* %i.i.i, align 4
  br label %for.cond.i.i

for.cond.i.i:                                     ; preds = %for.body.i.i, %"?log@@YAMM@Z.exit"
  %220 = load i32, i32* %i.i.i, align 4
  %cmp.i.i = icmp slt i32 %220, 100
  br i1 %cmp.i.i, label %for.body.i.i, label %"?exp@@YAMM@Z.exit.i"

for.body.i.i:                                     ; preds = %for.cond.i.i
  %221 = load float, float* %term.i.i, align 4
  %222 = load float, float* %x.addr.i.i, align 4
  %mul.i.i = fmul float %221, %222
  %223 = load i32, i32* %i.i.i, align 4
  %conv.i.i = sitofp i32 %223 to float
  %div.i.i = fdiv float %mul.i.i, %conv.i.i
  store float %div.i.i, float* %term.i.i, align 4
  %224 = load float, float* %term.i.i, align 4
  %225 = load float, float* %sum.i.i, align 4
  %add.i.i = fadd float %225, %224
  store float %add.i.i, float* %sum.i.i, align 4
  %226 = load i32, i32* %i.i.i, align 4
  %inc.i.i = add nsw i32 %226, 1
  store i32 %inc.i.i, i32* %i.i.i, align 4
  br label %for.cond.i.i, !llvm.loop !6

"?exp@@YAMM@Z.exit.i":                            ; preds = %for.cond.i.i
  %227 = load float, float* %sum.i.i, align 4
  store float %227, float* %retval.i, align 4
  br label %"?pow@@YAMMM@Z.exit"

"?pow@@YAMMM@Z.exit":                             ; preds = %if.then.i, %"?exp@@YAMM@Z.exit.i"
  %228 = load float, float* %retval.i, align 4
  store float %228, float* %d, align 4
  %229 = load float, float* %colx, align 4
  %230 = load float, float* %d, align 4
  %mul37 = fmul float %229, %230
  %231 = load float*, float** %Cr.addr, align 8
  %232 = load float, float* %231, align 4
  %add38 = fadd float %232, %mul37
  store float %add38, float* %231, align 4
  %233 = load float, float* %coly, align 4
  %234 = load float, float* %d, align 4
  %mul39 = fmul float %233, %234
  %235 = load float*, float** %Cg.addr, align 8
  %236 = load float, float* %235, align 4
  %add40 = fadd float %236, %mul39
  store float %add40, float* %235, align 4
  %237 = load float, float* %colz, align 4
  %238 = load float, float* %d, align 4
  %mul41 = fmul float %237, %238
  %239 = load float*, float** %Cb.addr, align 8
  %240 = load float, float* %239, align 4
  %add42 = fadd float %240, %mul41
  store float %add42, float* %239, align 4
  br label %for.inc

for.inc:                                          ; preds = %"?pow@@YAMMM@Z.exit"
  %241 = load float, float* %i, align 4
  %inc = fadd float %241, 1.000000e+00
  store float %inc, float* %i, align 4
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 15.0.0 (https://github.com/intel/llvm 5352b423f8913fce44e8276de68a39ffdb190bdc)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
!6 = distinct !{!6, !5}
!7 = distinct !{!7, !5}
!8 = distinct !{!8, !5}
