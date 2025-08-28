import cpp

// from Initializer ir, int i_1, int i_2
// where ir.getDeclaration().getDefinition().getType().toString() = "TEEC_UUID" and
//     i_1 < ir.getExpr().getNumChild() and
//     i_2 < ir.getExpr().getChild(ir.getExpr().getNumChild() - 1).getNumChild()
// select ir.getExpr().getChild(i_1), ir.getExpr().getChild(ir.getExpr().getNumChild() - 1).getChild(i_2)

from string res, int i, int j
where 
    exists (Initializer ir | 
        ir.getDeclaration().getDefinition().getType().toString() = "TEEC_UUID" and 
        i < ir.getExpr().getNumChild() and
        res = ir.getExpr().getChild(i).toString() and
        j = 0
    ) or 
    exists( Initializer ir | 
        ir.getDeclaration().getDefinition().getType().toString() = "TEEC_UUID" and 
        i < ir.getExpr().getChild(ir.getExpr().getNumChild() - 1).getNumChild() and
        res = ir.getExpr().getChild(ir.getExpr().getNumChild() - 1).getChild(i).toString() and
        j = 1
    )
select res, i, j
