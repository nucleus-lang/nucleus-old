; ModuleID = 'Nucleus'
source_filename = "Nucleus"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

declare i32 @print(ptr)

declare i32 @printi(i32)

define i32 @main(i32 %argc, ptr %argv) !dbg !3 {
entry:
  call void @llvm.dbg.value(metadata i32 %argc, metadata !8, metadata !DIExpression()), !dbg !10
  call void @llvm.dbg.value(metadata ptr %argv, metadata !9, metadata !DIExpression()), !dbg !10
  %arrayalloc = alloca [9 x i8], align 1
  store i8 -16, ptr %arrayalloc, align 1
  %.fca.1.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 1
  store i8 -97, ptr %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 2
  store i8 -90, ptr %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 3
  store i8 -88, ptr %.fca.3.gep, align 1
  %.fca.4.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 4
  store i8 -16, ptr %.fca.4.gep, align 1
  %.fca.5.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 5
  store i8 -97, ptr %.fca.5.gep, align 1
  %.fca.6.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 6
  store i8 -90, ptr %.fca.6.gep, align 1
  %.fca.7.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 7
  store i8 -88, ptr %.fca.7.gep, align 1
  %.fca.8.gep = getelementptr inbounds [9 x i8], ptr %arrayalloc, i64 0, i64 8
  store i8 0, ptr %.fca.8.gep, align 1
  %calltmp = call i32 @print(ptr nonnull %arrayalloc)
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0}
!llvm.dbg.cu = !{!1}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = distinct !DICompileUnit(language: DW_LANG_C, file: !2, producer: "Nucleus Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!2 = !DIFile(filename: "main.nk", directory: ".")
!3 = distinct !DISubprogram(name: "main", scope: !2, file: !2, type: !4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !1, retainedNodes: !7)
!4 = !DISubroutineType(types: !5)
!5 = !{!6, !6, !6}
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_float)
!7 = !{!8, !9}
!8 = !DILocalVariable(name: "argc", scope: !3, file: !2, type: !6)
!9 = !DILocalVariable(name: "argv", arg: 1, scope: !3, file: !2)
!10 = !DILocation(line: 0, scope: !3)
