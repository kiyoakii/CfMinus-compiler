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
  %op42 = sitofp i32 %op41 to float
  %op43 = fadd float %op42, 0x3fe0000000000000
  br label %label3
label3:                                                ; preds = %label_entry, %label60%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry%label_entry
  %op61 = phi i32 [ 0, %label_entry ], [ %op63, %label60 ]
  %op62 = phi i32 [ 0, %label_entry ], [ %op64, %label60 ]
  %op44 = sitofp i32 %op61 to float
  %op45 = fcmp ult float %op44,%op43
  %op46 = zext i1 %op45 to i32
  %op47 = icmp ne i32 %op46, 0
  br i1 %op47, label %label48, label %label55
label48:                                                ; preds = %label3
  %op50 = add i32 %op62, 1
  %op52 = icmp eq i32 %op50, 1000
  %op53 = zext i1 %op52 to i32
  %op54 = icmp ne i32 %op53, 0
  br i1 %op54, label %label57, label %label60
label55:                                                ; preds = %label3
  call void @output(i32 %op61)
  ret void
label57:                                                ; preds = %label48
  %op59 = add i32 %op61, 1
  br label %label60
label60:                                                ; preds = %label48, %label57
  %op63 = phi i32 [ %op61, %label48 ], [ %op59, %label57 ]
  %op64 = phi i32 [ %op50, %label48 ], [ 0, %label57 ]
  br label %label3
}
