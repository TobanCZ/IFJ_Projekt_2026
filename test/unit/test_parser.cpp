#include <gtest/gtest.h>
#include <cstdio>

extern "C" {
    #include "parser.h"
    #include "scanner.h"
    #include "symtable.h"
}

// Pomocna funkce: podstrci kod na stdin, vyresetuje stav a spusti parser
static int run_parser(const char *code) {
    FILE *tmp = tmpfile();
    if (!tmp) {
        return INTERNAL_ERROR;
    }
    fputs(code, tmp);
    rewind(tmp);

    FILE *old_stdin = stdin;
    stdin = tmp;

    int res = parse();

    stdin = old_stdin;
    fclose(tmp);
    return res;
}

// ============================================================
// 1. ZAKLADNI PROGRAMY, IMPORT A @main
// ============================================================

TEST(ProgramHeader, ValidFactorial) {
    const char *code = R"(import ifj26.*

def factorial(n: Int): Int =
    var result = 1
    var i: Int = n // komentar
    while i > 1 do
        result = result * i
        i = i - 1
    result

@main def main(): Unit =
    val n = readInt()
    val result = factorial(n)
    print(result)
    print("\n")
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(ProgramHeader, MinimalOneLineMain) {
    const char *code = R"(import ifj26.*
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(ProgramHeader, MainAnnotationOnSeparateLine) {
    const char *code = R"(import ifj26.*
@main
def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(ProgramHeader, MissingImportFails) {
    const char *code = R"(@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeader, WrongImportNameFails) {
    const char *code = R"(import ifj25.*
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeader, MissingMainFails) {
    const char *code = R"(import ifj26.*
def helper(x: Int): Int = x + 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(ProgramHeader, DuplicateMainFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit = ()
@main def main2(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_REDEFINITION);
}

TEST(ProgramHeader, MainWithParamFails) {
    const char *code = R"(import ifj26.*
@main def main(x: Int): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(ProgramHeader, MainWithWrongReturnTypeFails) {
    const char *code = R"(import ifj26.*
@main def main(): Int = 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(ProgramHeader, FuncWithoutReturnTypeOk) {
    const char *code = R"(import ifj26.*
def add(a: Int, b: Int) = a + b
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(ProgramHeader, TrailingCommaInParamsFails) {
    const char *code = R"(import ifj26.*
def f(a: Int, ): Int = a
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 2. BLOKY, PRIKAZY A VOLANI FUNKCI
// ============================================================

TEST(Statements, SemicolonSeparatedStatementsOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val a = 1; var b: Double = 2.5; b = 3.0
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Statements, IfElseAndNestedWhileOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    var x = 10
    if x > 0 then
        while x > 0 do
            x = x - 1
    else
        x = 0
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Statements, IfWithoutElseFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < 2 then
        val a = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Statements, EmptyBlockFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 == 1 then
    else
        val a = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Statements, ReturnWithAndWithoutExprOk) {
    const char *code = R"(import ifj26.*
def f(x: Int): Int =
    return x + 1
@main def main(): Unit =
    return
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Statements, ValWithoutInitFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x: Int
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Statements, CallWithMultipleAndUnitArgsOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    foo(1, 2.5, "ahoj", x, ())
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(Statements, CallWithTrailingCommaFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    foo(1, 2, )
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 3. VYRAZY (PRECEDENCNI ANALYZA - PSA)
// ============================================================

TEST(Expressions, ComplexPrecedenceAndParensOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = ((1 + 2) * (3 - 4)) / 5
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Expressions, UnaryMinusCombinationsOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val a = -5
    val b = --5
    val c = -(a + b)
    val d = a * -b
    val e = a - -b
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Expressions, UnitLiteralInExprOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val u: Unit = ()
    return ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Expressions, RelationalAndEqualityOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val a = 5
    val b = 10
    if a + 1 <= b * 2 then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(Expressions, TrailingRightParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val result = factorial)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, UnclosedLeftParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = (1 + 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, MissingRightOperandFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1 +
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, LeadingBinaryOperatorFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = * 5
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, TwoBinaryOperatorsInRowFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1 + * 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, TwoOperandsInRowFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, ChainedRelationalFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < 2 < 3 then
        val a = 1
    else
        val a = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, ChainedEqualityFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 == 2 == 3 then
        val a = 1
    else
        val a = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, FuncCallInsideExprFailsInBaseIFJ26) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = factorial(n) * 5 + 55
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(Expressions, EmptyIfConditionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if then
        val a = 1
    else
        val a = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 5. EDGE CASY - HLAVICKA A STRUKTURA PROGRAMU
// ============================================================

TEST(ProgramHeaderEdge, TopLevelStatementFails) {
    const char *code = R"(import ifj26.*
val x = 1
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeaderEdge, DuplicateImportFails) {
    const char *code = R"(import ifj26.*
import ifj26.*
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeaderEdge, TokenAfterImportOnSameLineFails) {
    const char *code = R"(import ifj26.* def f(): Unit = ()
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeaderEdge, MainAnnotationWithoutDefFails) {
    const char *code = R"(import ifj26.*
@main val x = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

/* TEST(ProgramHeaderEdge, UnknownAnnotationFails) {
    const char *code = R"(import ifj26.*
@foo def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
} */

TEST(ProgramHeaderEdge, MissingEqualsInDefFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit
    ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeaderEdge, MissingReturnTypeAfterColonFails) {
    const char *code = R"(import ifj26.*
def f(): = 1
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeaderEdge, MissingParamTypeAfterColonFails) {
    const char *code = R"(import ifj26.*
def f(a: ): Int = a
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ProgramHeaderEdge, NestedDefInsideFunctionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    def inner(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 6. EDGE CASY - PRIKAZY A BLOKY
// ============================================================

TEST(StatementsEdge, TwoStatementsWithoutSemicolonFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val a = 1 val b = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, IfWithoutThenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < 2
        val a = 1
    else
        val a = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, WhileWithoutDoFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    while 1 < 2
        val a = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, StandaloneElseFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    else
        val a = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, KeywordAsVarNameFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val if = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, KeywordAsFuncNameFails) {
    const char *code = R"(import ifj26.*
def while(): Unit = ()
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, AssignToLiteralFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    5 = 10
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, AssignToExprFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    (a + b) = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, StatementAfterReturnFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    return val x = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, CallUnclosedParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    foo(1, 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, CallExtraRightParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    foo(1, 2))
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, CallLeadingCommaFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    foo(, 1)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(StatementsEdge, CallDoubleCommaFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    foo(1,, 2)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 7. EDGE CASY - VYRAZY (PSA)
// ============================================================

TEST(ExpressionsEdge, UnaryPlusFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = +5
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, UnaryPlusAfterOperatorFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1 + +2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, DanglingUnaryMinusFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = -
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, DanglingUnaryMinusInParensFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = (-)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, AssignInsideIfConditionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if a = 5 then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, AssignInsideParensFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = (a = 1)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, ImplicitMultiplicationBeforeParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 2(3 + 4)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, ImplicitMultiplicationAfterParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = (1 + 2)3
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, ImplicitMultiplicationTwoParensFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = (1 + 2)(3 + 4)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, OperatorBeforeRightParenFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = (1 + )
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, EmptyWhileConditionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    while do
        val a = 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, TwoUnitLiteralsInRowFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = () ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(ExpressionsEdge, OperandFollowedByUnitLiteralFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1 ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 8. SEMANTIKA - NEDEFINOVANE SYMBOLY (CHYBA 3)
// ============================================================

TEST(SemanticsUndefined, UndefinedVarInExprFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = y + 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, UndefinedVarOnAssignLhsFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    x = 5
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, UndefinedVarInSelfInitFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = x + 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, UndefinedFunctionCallFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    neexistuje(1)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, UndefinedArgInFunctionCallFails) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit = ()
@main def main(): Unit =
    f(neexistuje)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, VarOutOfIfScopeFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < 2 then
        val tajna = 42
    else
        val jina = 10
    val x = tajna
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, VarOutOfWhileScopeFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    while 1 > 2 do
        var lokalni = 5
    lokalni = 10
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, ParamOutOfFunctionScopeFails) {
    const char *code = R"(import ifj26.*
def helper(tajnyParam: Int): Unit = ()
@main def main(): Unit =
    val x = tajnyParam
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsUndefined, CallNonFunctionAsFunctionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 5
    x()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

// ============================================================
// 9. SEMANTIKA - REDEFINICE A ZASTINENI (CHYBA 4 + OK)
// ============================================================

TEST(SemanticsRedefinition, DuplicateVarInSameScopeFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1
    var x = 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_REDEFINITION);
}

TEST(SemanticsRedefinition, DuplicateParamNamesFails) {
    const char *code = R"(import ifj26.*
def f(a: Int, a: Double): Unit = ()
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_REDEFINITION);
}

TEST(SemanticsRedefinition, LocalVarSameNameAsParamFails) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit =
    val a = 10
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_REDEFINITION);
}

TEST(SemanticsRedefinition, DuplicateFunctionNamesFails) {
    const char *code = R"(import ifj26.*
def foo(): Unit = ()
def foo(x: Int): Int = x
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_REDEFINITION);
}

TEST(SemanticsRedefinition, RedefineBuiltinFunctionFails) {
    const char *code = R"(import ifj26.*
def print(x: Int): Unit = ()
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_REDEFINITION);
}

TEST(SemanticsRedefinition, ShadowingInNestedIfBlockOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 10
    if x > 5 then
        val x = "ahoj"
        print(x)
    else
        val x = 3.14
        print(x)
    val y: Int = x + 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsRedefinition, SameVarNameInSiblingElseBlockOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < 2 then
        val x = 1
    else
        val x = "jiný typ v else větvi"
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

// ============================================================
// 10. SEMANTIKA - POCET A TYPY ARGUMENTU (CHYBA 5)
// ============================================================

TEST(SemanticsArguments, TooFewArgumentsFails) {
    const char *code = R"(import ifj26.*
def f(a: Int, b: Int): Unit = ()
@main def main(): Unit =
    f(1)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, TooManyArgumentsFails) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit = ()
@main def main(): Unit =
    f(1, 2)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, BuiltinPrintWithZeroArgsFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    print()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, BuiltinPrintWithTwoArgsFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    print(1, 2)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, WrongArgTypeStringToIntFails) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit = ()
@main def main(): Unit =
    f("text")
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, NarrowingArgDoubleToIntFails) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit = ()
@main def main(): Unit =
    f(3.14)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, UnitArgToIntParamFails) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit = ()
@main def main(): Unit =
    f(())
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsArguments, WideningArgIntToDoubleOk) {
    const char *code = R"(import ifj26.*
def f(a: Double): Unit = ()
@main def main(): Unit =
    f(42)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsArguments, ForwardCallBeforeDefinitionOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val r: Double = dopredna(5, 2.0)

def dopredna(a: Int, b: Double): Double = a + b
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

// ============================================================
// 11. SEMANTIKA - TYPOVA KOMPATIBILITA (CHYBA 6)
// ============================================================

TEST(SemanticsTypes, InitIntWithDoubleFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x: Int = 3.14
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, InitIntWithStringFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x: Int = "ahoj"
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, InitStringWithIntFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val s: String = 123
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, InitUnitWithIntFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val u: Unit = 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, InitDoubleWithIntOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val d: Double = 10
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsTypes, AssignWrongTypeToInferredVarFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    var x = 10
    x = "text"
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, AssignDoubleToInferredIntVarFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    var x = 10
    x = 2.5
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, AssignIntToInferredDoubleVarOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    var x = 10.5
    x = 3
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsTypes, StoreRelationalResultInValFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val b = 1 < 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, AssignRelationalResultToVarFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    var x = 1
    x = 1 == 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, ReturnRelationalResultFails) {
    const char *code = R"(import ifj26.*
def f(): Int =
    return 1 < 2
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, AddStringAndIntFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = "a" + 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, SubtractStringsFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = "a" - "b"
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, MultiplyStringByIntFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = "a" * 3
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, UnaryMinusOnStringFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = -"ahoj"
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, UnaryMinusOnUnitFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = -()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, ArithmeticWithUnitFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = () + 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, CompareIntAndStringWithLessFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < "a" then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, CompareIntAndStringWithEqualFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 == "1" then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, CompareIntAndDoubleOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 < 2.5 then
        if 3.0 == 3 then
            val x = 1
        else
            val x = 2
    else
        val x = 3
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsTypes, CompareStringsAndUnitsOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if "abc" < "def" then
        if () == () then
            val x = 1
        else
            val x = 2
    else
        val x = 3
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsTypes, StringConcatenationAndIntToDoublePromotionOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val s: String = "ahoj " + "svete"
    val d: Double = (1 + 2) * 3.5 - 4 / 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsTypes, NonBooleanIntInIfConditionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if 1 then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, NonBooleanStringInWhileConditionFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    while "true" do
        val x = 1
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsTypes, AssignVoidFunctionCallToIntVarFails) {
    const char *code = R"(import ifj26.*
def nic(): Unit = ()
@main def main(): Unit =
    val x: Int = nic()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

// ============================================================
// 12. SEMANTIKA - ZAPIS DO NEMENNE PROMENNE (CHYBA 7)
// ============================================================

TEST(SemanticsImmutable, ReassignValFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 10
    x = 20
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_WRITE);
}

TEST(SemanticsImmutable, ReassignFunctionParamFails) {
    const char *code = R"(import ifj26.*
def f(p: Int): Unit =
    p = 99
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_WRITE);
}

TEST(SemanticsImmutable, AssignToFunctionNameFails) {
    const char *code = R"(import ifj26.*
def f(): Unit = ()
@main def main(): Unit =
    f = 5
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_WRITE);
}

// ============================================================
// 13. SEMANTIKA - NAVRATOVE HODNOTY FUNKCI (CHYBA 10 / OTHER)
// ============================================================

TEST(SemanticsReturn, EmptyReturnInIntFunctionFails) {
    const char *code = R"(import ifj26.*
def f(): Int =
    return
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_OTHER);
}

TEST(SemanticsReturn, ValueReturnInUnitFunctionFails) {
    const char *code = R"(import ifj26.*
def f(): Unit =
    return 42
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsReturn, StringReturnInIntFunctionFails) {
    const char *code = R"(import ifj26.*
def f(): Int =
    return "spatne"
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsReturn, DoubleReturnInIntFunctionFails) {
    const char *code = R"(import ifj26.*
def f(): Int =
    return 1.5
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsReturn, IntReturnInDoubleFunctionOk) {
    const char *code = R"(import ifj26.*
def f(): Double =
    return 5
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

// ============================================================
// 14. SEMANTIKA - PRIORITA CHYB (SYNTAX 2 > SEMANTIKA 3..7)
// ============================================================

TEST(SemanticsPrecedence, SyntaxErrorBeatsUndefinedVarInExpr) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = neexistuje + * 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(SemanticsPrecedence, SyntaxErrorBeatsTypeMismatchInExpr) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = ("ahoj" + 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(SemanticsPrecedence, UndefinedVarBeatsTypeMismatchInExpr) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = neexistuje + "ahoj"
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsPrecedence, UndefinedFuncBeatsWrongArgCount) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    neexistuje(1, 2, 3)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

// ============================================================
// 15. SEMANTIKA - VESTAVENE FUNKCE A KONVERZE
// ============================================================

TEST(SemanticsBuiltins, BuiltinConversionFunctionsOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val s: String = readLine()
    val i: Int = readInt()
    val d: Double = readDouble()
    val asStr: String = toStr(d)
    val asInt: Int = toInt(d)
    val len: Int = strLength(s)
    val sub: String = subStr(s, 0, len)
    val codeChar: Int = ord(s, 0)
    val ch: String = chr(codeChar)
    print(ch)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsBuiltins, ToIntWithIntArgWidensToDoubleOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x: Int = toInt(5)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsBuiltins, ToIntWithStringArgFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x: Int = toInt("123")
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsBuiltins, SubStrWithDoubleIndexFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val s = subStr("ahoj", 1.5, 2)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsBuiltins, OrdWithWrongArgOrderFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val c = ord(0, "ahoj")
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsBuiltins, ChrWithStringFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val c = chr("A")
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

TEST(SemanticsBuiltins, ReadLineWithArgFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val s = readLine("prompt")
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_INVALID_ARGUMENT_COUNT);
}

// ============================================================
// 16. SEMANTIKA - ZASTINENI FUNKCE PROMENNOU A REKURZE
// ============================================================

TEST(SemanticsShadowing, LocalVarWithSameNameAsFunctionCallStillWorks) {
    const char *code = R"(import ifj26.*
def helper(x: Int): Int = x + 1
@main def main(): Unit =
    val helper = 10
    val res: Int = helper(helper)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsShadowing, LocalVarWithSameNameAsFunctionIsMutableIfVar) {
    const char *code = R"(import ifj26.*
def helper(): Unit = ()
@main def main(): Unit =
    var helper = 1
    helper = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsShadowing, MutualRecursionOk) {
    const char *code = R"(import ifj26.*
def isEven(n: Int): Int =
    if n == 0 then
        return 1
    else
        val prev = n - 1
        val r = isOdd(prev)
        return r

def isOdd(n: Int): Int =
    if n == 0 then
        return 0
    else
        val prev = n - 1
        val r = isEven(prev)
        return r

@main def main(): Unit =
    val x = isEven(4)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

// ============================================================
// 17. SEMANTIKA - JEDNORADKOVE FUNKCE A ODVOZOVANI TYPU
// ============================================================

TEST(SemanticsOneLiner, OneLineFuncInfersDoubleAndChecksCallersOk) {
    const char *code = R"(import ifj26.*
def half(x: Int) = x / 2.0
@main def main(): Unit =
    val d: Double = half(5)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsOneLiner, OneLineFuncInferredDoubleAssignedToIntFails) {
    const char *code = R"(import ifj26.*
def half(x: Int) = x / 2.0
@main def main(): Unit =
    val i: Int = half(5)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsOneLiner, OneLineFuncWithRelationalBodyFails) {
    const char *code = R"(import ifj26.*
def isPositive(x: Int) = x > 0
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsOneLiner, OneLineFuncDeclaredIntWithDoubleBodyFails) {
    const char *code = R"(import ifj26.*
def bad(x: Int): Int = x + 1.5
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsOneLiner, OneLineFuncDeclaredDoubleWithIntBodyOk) {
    const char *code = R"(import ifj26.*
def promote(x: Int): Double = x + 1
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

// ============================================================
// 18. SEMANTIKA - SLOZITEJSI VYRAZY A NESTED SCOPES
// ============================================================

TEST(SemanticsComplex, DeeplyNestedScopesAndTypesOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    var sum: Double = 0
    var i = 0
    while i < 5 do
        val step = 1.5
        if i == 2 then
            val step = 10
            sum = sum + step
        else
            sum = sum + step
        i = i + 1
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsComplex, EqualityBetweenDoubleAndIntOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val i = 5
    val d = 5.0
    if i == d then
        if d != 3 then
            val ok = 1
        else
            val ok = 2
    else
        val ok = 3
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsComplex, EqualityBetweenUnitAndIntFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if () == 0 then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsComplex, LessThanBetweenUnitsFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if () < () then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsComplex, ReturnInsideIfAndWhileBranchesOk) {
    const char *code = R"(import ifj26.*
def findFirst(limit: Int): Double =
    var i = 0
    while i < limit do
        if i == 3 then
            return i
        else
            i = i + 1
    return 0.0

@main def main(): Unit =
    val r: Double = findFirst(10)
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsComplex, WrongReturnTypeInsideNestedIfFails) {
    const char *code = R"(import ifj26.*
def broken(flag: Int): Int =
    if flag == 1 then
        return "chyba uvnitr if"
    else
        return 0

@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

// ============================================================
// 19. SEMANTIKA - PRIORITA CHYB V PODMINKACH A VOLANICH
// ============================================================

TEST(SemanticsPrecedence, SyntaxErrorInIfConditionBeatsUndefinedVar) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    if neexistuje + then
        val x = 1
    else
        val x = 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(SemanticsPrecedence, SyntaxErrorInCallArgsBeatsUndefinedFunc) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    neexistuje(1, )
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

TEST(SemanticsPrecedence, UndefinedArgBeatsWrongArgCount) {
    const char *code = R"(import ifj26.*
def f(a: Int): Unit = ()
@main def main(): Unit =
    f(neexistuje, 2)
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_UNDEFINED);
}

TEST(SemanticsPrecedence, UndefinedVarInAssignRhsBeatsImmutableLhs) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val x = 1
    x = neexistuje + * 2
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

// ============================================================
// 20. SEMANTIKA - REKURZE A ODVOZOVANI NAVRATOVEHO TYPU FUNKCI
// ============================================================

TEST(SemanticsFuncReturnInference, DirectRecursionWithoutReturnTypeFails) {
    const char *code = R"(import ifj26.*
def fact(n: Int) =
    if n <= 1 then
        1
    else
        val prev = n - 1
        val sub = fact(prev)
        n * sub

@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsFuncReturnInference, OneLineDirectRecursionWithoutReturnTypeFails) {
    const char *code = R"(import ifj26.*
def loop(n: Int) = loop(n)
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsFuncReturnInference, MutualRecursionWithoutReturnTypeFails) {
    const char *code = R"(import ifj26.*
def f(n: Int) = g(n)
def g(n: Int): Int = f(n)
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsFuncReturnInference, ReturnInsideFuncWithoutExplicitReturnTypeFails) {
    const char *code = R"(import ifj26.*
def f(x: Int) =
    return x + 1
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsFuncReturnInference, EmptyReturnInsideFuncWithoutExplicitReturnTypeFails) {
    const char *code = R"(import ifj26.*
def f() =
    return
@main def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}

TEST(SemanticsFuncReturnInference, ForwardCallToInferredFuncDefinedBelowMainOk) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val d: Double = laterFunc(4)

def laterFunc(x: Int) = x / 2.0
)";
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
}

TEST(SemanticsFuncReturnInference, ForwardCallToInferredFuncTypeMismatchFails) {
    const char *code = R"(import ifj26.*
@main def main(): Unit =
    val i: Int = laterFunc(4)

def laterFunc(x: Int) = x / 2.0
)";
    EXPECT_EQ(run_parser(code), SEM_ERROR_TYPE_MISMATCH);
}