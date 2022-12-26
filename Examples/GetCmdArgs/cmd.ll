; ModuleID = 'cmd.cpp'
source_filename = "cmd.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-w64-windows-gnu"

%"class.std::ios_base::Init" = type { i8 }

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@.str = private unnamed_addr constant [20 x i8] c"This is String One!\00", align 1
@.str.1 = private unnamed_addr constant [20 x i8] c"This is String Two!\00", align 1
@.str.2 = private unnamed_addr constant [22 x i8] c"This is String Three!\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_cmd.cpp, ptr null }]

; Function Attrs: noinline uwtable
define internal void @__cxx_global_var_init() #0 {
  call void @_ZNSt8ios_base4InitC1Ev(ptr noundef nonnull align 1 dereferenceable(1) @_ZStL8__ioinit)
  %1 = call i32 @atexit(ptr @__dtor__ZStL8__ioinit) #3
  ret void
}

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #2

; Function Attrs: noinline uwtable
define internal void @__dtor__ZStL8__ioinit() #0 {
  call void @_ZNSt8ios_base4InitD1Ev(ptr @_ZStL8__ioinit)
  ret void
}

; Function Attrs: nounwind
declare dso_local i32 @atexit(ptr) #3

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local noundef i32 @_Z11printStringPKc(ptr noundef %0) #4 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = call noundef i32 (ptr, ...) @_ZL6printfPKcz(ptr noundef %3)
  ret i32 0
}

; Function Attrs: mustprogress noinline optnone uwtable
define internal noundef i32 @_ZL6printfPKcz(ptr noundef nonnull %0, ...) #4 {
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  call void @llvm.va_start(ptr %4)
  %5 = call ptr @__acrt_iob_func(i32 noundef 1)
  %6 = load ptr, ptr %2, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = call i32 @__mingw_vfprintf(ptr noundef %5, ptr noundef %6, ptr noundef %7) #3
  store i32 %8, ptr %3, align 4
  call void @llvm.va_end(ptr %4)
  %9 = load i32, ptr %3, align 4
  ret i32 %9
}

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main(i32 noundef %0, ptr noundef %1) #5 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca [3 x ptr], align 16
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  store ptr @.str, ptr %6, align 8
  store ptr @.str.1, ptr %7, align 8
  store ptr @.str.2, ptr %8, align 8
  %10 = getelementptr inbounds [3 x ptr], ptr %9, i64 0, i64 0
  %11 = load ptr, ptr %6, align 8
  store ptr %11, ptr %10, align 8
  %12 = getelementptr inbounds ptr, ptr %10, i64 1
  %13 = load ptr, ptr %7, align 8
  store ptr %13, ptr %12, align 8
  %14 = getelementptr inbounds ptr, ptr %12, i64 1
  %15 = load ptr, ptr %8, align 8
  store ptr %15, ptr %14, align 8
  %16 = getelementptr inbounds [3 x ptr], ptr %9, i64 0, i64 0
  %17 = load ptr, ptr %16, align 16
  %18 = call noundef i32 @_Z11printStringPKc(ptr noundef %17)
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #6

; Function Attrs: nounwind
declare dso_local i32 @__mingw_vfprintf(ptr noundef, ptr noundef, ptr noundef) #2

declare dllimport ptr @__acrt_iob_func(i32 noundef) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #6

; Function Attrs: noinline uwtable
define internal void @_GLOBAL__sub_I_cmd.cpp() #0 {
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { noinline uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind }
attributes #4 = { mustprogress noinline optnone uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 15.0.5"}
