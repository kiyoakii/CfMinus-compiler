; ModuleID = 'cminus'
source_filename = "../tests/lab5/testcases/LoopInvHoist/testcase-6.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op6 = mul i32 5, 5
  br label %label2
label2:                                                ; preds = %label_entry, %label10%label_entry
  %op15 = phi i32 [ 0, %label_entry ], [ %op12, %label10 ]
  %op7 = icmp slt i32 %op15, %op6
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label13
label10:                                                ; preds = %label2
  %op12 = add i32 %op15, 1
  br label %label2
label13:                                                ; preds = %label2
  call void @output(i32 %op15)
  ret void
}
