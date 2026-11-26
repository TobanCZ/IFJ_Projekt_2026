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

    Symtable table;
    // Pokud mate inicializaci tabulky symbolu, odkomentuj:
    // symtable_init(&table);

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
    EXPECT_EQ(run_parser(code), SEM_ERROR_NON_DEFINE);
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
    EXPECT_EQ(run_parser(code), SYNTAX_OK);
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

TEST(ProgramHeaderEdge, UnknownAnnotationFails) {
    const char *code = R"(import ifj26.*
@foo def main(): Unit = ()
)";
    EXPECT_EQ(run_parser(code), SYNTAX_ERROR);
}

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