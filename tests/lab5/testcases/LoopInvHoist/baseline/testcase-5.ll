; ModuleID = 'cminus'
source_filename = "../tests/lab5/testcases/LoopInvHoist/testcase-5.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op7 = mul i32 2, 2
  %op9 = mul i32 %op7, 2
  %op11 = mul i32 %op9, 2
  %op13 = mul i32 %op11, 2
  %op15 = mul i32 %op13, 2
  %op17 = mul i32 %op15, 2
  %op19 = mul i32 %op17, 2
  %op21 = mul i32 %op19, 2
  %op23 = mul i32 %op21, 2
  %op25 = mul i32 %op23, 2
  %op27 = mul i32 %op25, 2
  %op29 = mul i32 %op27, 2
  %op31 = mul i32 %op29, 2
  %op33 = mul i32 %op31, 2
  %op35 = mul i32 %op33, 2
  %op37 = mul i32 %op35, 2
  %op39 = mul i32 %op37, 2
  %op41 = mul i32 %op39, 2
  br label %label3
label3:                                                ; preds = %label_entry, %label57%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry
  %op58 = phi i32 [ 0, %label_entry ], [ %op60, %label57 ]
  %op59 = phi i32 [ 0, %label_entry ], [ %op61, %label57 ]
  %op42 = icmp slt i32 %op58, %op41
  %op43 = zext i1 %op42 to i32
  %op44 = icmp ne i32 %op43, 0
  br i1 %op44, label %label45, label %label52
label45:                                                ; preds = %label3
  %op47 = add i32 %op59, 1
  %op49 = icmp eq i32 %op47, 1000
  %op50 = zext i1 %op49 to i32
  %op51 = icmp ne i32 %op50, 0
  br i1 %op51, label %label54, label %label57
label52:                                                ; preds = %label3
  call void @output(i32 %op58)
  ret void
label54:                                                ; preds = %label45
  %op56 = add i32 %op58, 1
  br label %label57
label57:                                                ; preds = %label45, %label54
  %op60 = phi i32 [ %op58, %label45 ], [ %op56, %label54 ]
  %op61 = phi i32 [ %op47, %label45 ], [ 0, %label54 ]
  br label %label3
}
