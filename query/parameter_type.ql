import cpp

from MacroInvocation mi
where mi.getMacroName().toString() = "TEE_PARAM_TYPES"
select mi.getExpandedArgument(0), mi.getExpandedArgument(1), mi.getExpandedArgument(2), mi.getExpandedArgument(3), mi.getExpr().getEnclosingFunction()
