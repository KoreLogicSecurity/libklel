/*-
 ***********************************************************************
 *
 * $Id: parser.c,v 1.73 2012/11/14 02:43:14 klm Exp $
 *
 ***********************************************************************
 *
 * Copyright 2011-2012 The KL-EL Project, All Rights Reserved.
 *
 ***********************************************************************
 */
#include "all-includes.h"

/*-
 ***********************************************************************
 *
 * KlelCopyProduction
 * Copy a captured production. Note that this is a deep copy.
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCopyProduction(KLEL_NODE *psNode)
{
  KLEL_NODE *psCopy = NULL;
  KLEL_NODE *psTemp = NULL;
  size_t    szi     = 0;

  if (psNode != NULL)
  {
    psCopy = calloc(1, sizeof(KLEL_NODE));

    if (psCopy != NULL)
    {
      memcpy(psCopy, psNode, sizeof(KLEL_NODE));
      psCopy->iReferenceCount = 0;

      for (szi = 0; szi < KLEL_MAX_CHILDREN; szi++)
      {
        if (psCopy->apsChildren[szi] != NULL)
        {
          psTemp = KlelCopyProduction(psCopy->apsChildren[szi]);
          if (psTemp == NULL)
          {
            psCopy->apsChildren[szi] = NULL;
            KlelFreeNode(psCopy);
            return NULL;
          }
          psCopy->apsChildren[szi] = psTemp;
        }
      }
    }
  }

  return psCopy;
}


/*-
 ***********************************************************************
 *
 * KlelCapturedProduction
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCapturedProduction(const char *pcName, KLEL_PRODUCTION_TOKEN_MAP *psMap, size_t szMapSize)
{
  size_t    szi             = 0;
  KLEL_NODE *psReturnedNode = NULL;

  KLEL_ASSERT(psMap     != NULL);
  KLEL_ASSERT(szMapSize  > 0);

  for (szi = 0; szi < szMapSize && psReturnedNode == NULL; szi++)
  {
    if (psMap[szi].pcName != NULL && strcmp(psMap[szi].pcName, pcName) == 0)
    {
      psReturnedNode = psMap[szi].psToken;
    }
  }

  return psReturnedNode;
}


/*-
 ***********************************************************************
 *
 * KlelClosureForDesignator
 *
 ***********************************************************************
 */
int
KlelClosureForDesignator(KLEL_CONTEXT *psContext, const char *pcName)
{
  KLEL_CLOSURE *psEnvironment = NULL;

  KLEL_ASSERT(psContext != NULL);
  KLEL_ASSERT(pcName    != NULL);
  KLEL_ASSERT(pcName[0] != 0);

  for (psEnvironment = psContext->psEnvironment; psEnvironment != NULL; psEnvironment = psEnvironment->psNext)
  {
    if (strncmp(pcName, psEnvironment->acVariable, KLEL_MAX_NAME) == 0)
    {
      return psEnvironment->iIndex;
    }
  }

  return -1;
}


/*-
 ***********************************************************************
 *
 * KlelCreateNode
 *
 ***********************************************************************
 */
KLEL_NODE *
KlelCreateNode(KLEL_NODE_TYPE iType)
{
  KLEL_NODE *psNode = calloc(1, sizeof(KLEL_NODE));
  if (psNode != NULL)
  {
    psNode->iType = iType;
  }

  return psNode;
}


/*-
 ***********************************************************************
 *
 * KlelReleaseNode
 *
 ***********************************************************************
 */
void
KlelReleaseNode(KLEL_NODE *psNode)
{
  int iChild = 0;

  KLEL_ASSERT(psNode != NULL);

  if (psNode != NULL)
  {
    psNode->iReferenceCount--;

    if (psNode->iReferenceCount < 0)
    {
      for (iChild = 0; iChild < KLEL_MAX_CHILDREN; iChild++)
      {
        if (psNode->apsChildren[iChild] != NULL)
        {
          KlelReleaseNode(psNode->apsChildren[iChild]);
        }
      }

      free(psNode);
    }
  }
}


/*-
 ***********************************************************************
 *
 * KlelRetainNode
 *
 ***********************************************************************
 */
void
KlelRetainNode(KLEL_NODE *psNode)
{
  KLEL_ASSERT(psNode != NULL);

  if (psNode != NULL)
  {
    psNode->iReferenceCount++;
  }
}


/*-
 ***********************************************************************
 *
 * KlelStripSpace
 *
 ***********************************************************************
 */
const char *
KlelStripSpace(const char *pcInput)
{
  KLEL_ASSERT(pcInput != NULL);

  while (isspace(pcInput[0]))
  {
    pcInput++;
  }

  return pcInput;
}


/*-
 ***********************************************************************
 *
 * The Node-to-String Map, Used for Error Messages
 *
 ***********************************************************************
 */
char *apcKlelNodeToString[] =
{
  "error",
  "operator",
  "operator",
  "operator",
  "function call",
  "operator",
  ")",
  "]",
  ":",
  ",",
  "expression",
  "designator",
  "string interpolation sigil",
  "operator",
  "end of input",
  "=",
  "operator",
  "keyword 'eval'",
  "expression",
  "keyword 'fail'",
  "string",
  "guarded command",
  "operator",
  "operator",
  "operator",
  "keyword 'if'",
  "keyword 'in'",
  "integer",
  "string interpolation",
  "keyword 'let'",
  "operator",
  "operator",
  "operator",
  "operator",
  "operator",
  "operator",
  "operator",
  "(",
  "[",
  "keyword 'pass'",
  "operator",
  "operator",
  "operator",
  "operator",
  "?",
  "\"",
  "quoted interpolation",
  "real",
  "operator",
  "operator",
  "keyword 'then'",
  "operator",
  "operator"
};


/*-
 ***********************************************************************
 *
 * KlelRoot <-- [Designator ':'] (GuardedCommand | Expression)
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelRoot)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Colon)
    KLEL_TOKEN_DEF(Designator)
    KLEL_TOKEN_DEF(GuardedCommand)
    KLEL_TOKEN_DEF(Eof)
    KLEL_TOKEN_DEF(Expression)
  KLEL_END_PRODUCTION_TOKENS

  if (KLEL_PEEK() == KLEL_NODE_DESIGNATOR && KLEL_PEEK2() == KLEL_NODE_COLON)
  {
    KLEL_CAPTURE_PRODUCTION(Designator, KLEL_EXPECT(KLEL_NODE_DESIGNATOR));
    KLEL_CAPTURE_PRODUCTION(Colon, KLEL_EXPECT(KLEL_NODE_COLON));
    KLEL_ADD_TO_RETURNED_NODE(KLEL_LABEL_INDEX, Designator);
  }

  if (KLEL_PEEK() == KLEL_NODE_IF)
  {
    KLEL_CAPTURE_PRODUCTION(GuardedCommand, KLEL_INVOKE(KlelGuardedCommand));
    KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_GUARDED_COMMAND);
    KLEL_ADD_TO_RETURNED_NODE(KLEL_EXPRESSION_INDEX, GuardedCommand);
  }
  else
  {
    KLEL_CAPTURE_PRODUCTION(Expression, KLEL_INVOKE(KlelExpression));
    KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_EXPRESSION);
    KLEL_ADD_TO_RETURNED_NODE(KLEL_EXPRESSION_INDEX, Expression);
  }

  KLEL_CAPTURE_PRODUCTION(Eof, KLEL_EXPECT(KLEL_NODE_EOF));
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * GuardedCommand <-- 'if' '(' Expression ')' 'then' 'eval'
 *                    '(' String ',' String {',' Expression} ')'
 *                    [('pass' | 'fail') '[' Integer {',' Integer} ']'
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelGuardedCommand)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Argument)
    KLEL_TOKEN_DEF(Arg0)
    KLEL_TOKEN_DEF(Command)
    KLEL_TOKEN_DEF(CloseParen1)
    KLEL_TOKEN_DEF(CloseParen2)
    KLEL_TOKEN_DEF(CloseSquare)
    KLEL_TOKEN_DEF(Code)
    KLEL_TOKEN_DEF(Codes)
    KLEL_TOKEN_DEF(Comma)
    KLEL_TOKEN_DEF(Eval)
    KLEL_TOKEN_DEF(If)
    KLEL_TOKEN_DEF(Interpreter)
    KLEL_TOKEN_DEF(OpenParen1)
    KLEL_TOKEN_DEF(OpenParen2)
    KLEL_TOKEN_DEF(OpenSquare)
    KLEL_TOKEN_DEF(PassOrFail)
    KLEL_TOKEN_DEF(Predicate)
    KLEL_TOKEN_DEF(Then)
  KLEL_END_PRODUCTION_TOKENS

  int        iCodesIndex    = 0;
  int        iArgumentCount = 0;

  KLEL_CAPTURE_PRODUCTION(If,          KLEL_EXPECT(KLEL_NODE_IF));
  KLEL_CAPTURE_PRODUCTION(OpenParen1,  KLEL_EXPECT(KLEL_NODE_OPEN_PAREN));
  KLEL_CAPTURE_PRODUCTION(Predicate,   KLEL_INVOKE(KlelExpression));
  KLEL_CAPTURE_PRODUCTION(CloseParen1, KLEL_EXPECT(KLEL_NODE_CLOSE_PAREN));
  KLEL_CAPTURE_PRODUCTION(Then,        KLEL_EXPECT(KLEL_NODE_THEN));
  KLEL_CAPTURE_PRODUCTION(Eval,        KLEL_EXPECT(KLEL_NODE_EVAL));
  KLEL_CAPTURE_PRODUCTION(OpenParen2,  KLEL_EXPECT(KLEL_NODE_OPEN_PAREN));
  KLEL_CAPTURE_PRODUCTION(Interpreter, KLEL_INVOKE(KlelString));
  KLEL_CAPTURE_PRODUCTION(Comma,       KLEL_EXPECT(KLEL_NODE_COMMA));
  KLEL_CAPTURE_PRODUCTION(Command,     KLEL_INVOKE(KlelString));

  KLEL_CAPTURE_PRODUCTION(Arg0, KLEL_COPY_PRODUCTION(Command)); /* ppcArgumentVector[0] is also the command for use with execv(). */
  KLEL_ADD_TO_RETURNED_NODE(iArgumentCount, Arg0);
  iArgumentCount++;

  while (KLEL_PEEK() == KLEL_NODE_COMMA)
  {
    if (iArgumentCount >= KLEL_MAX_FUNC_ARGS)
    {
      KLEL_REPORT_ERROR("too many arguments to eval() function");
    }
    KLEL_CAPTURE_PRODUCTION(Comma, KLEL_EXPECT(KLEL_NODE_COMMA));
    KLEL_CAPTURE_PRODUCTION(Argument, KLEL_INVOKE(KlelExpression));
    KLEL_ADD_TO_RETURNED_NODE(iArgumentCount, Argument);
    iArgumentCount++;
  }
  KLEL_CAPTURE_PRODUCTION(CloseParen2, KLEL_EXPECT(KLEL_NODE_CLOSE_PAREN));

  if (KLEL_PEEK() == KLEL_NODE_PASS || KLEL_PEEK() == KLEL_NODE_FAIL)
  {
    iCodesIndex = (KLEL_PEEK() == KLEL_NODE_PASS) ? KLEL_PASS_INDEX : KLEL_FAIL_INDEX;
    KLEL_CAPTURE_PRODUCTION(PassOrFail, KLEL_EXPECT(KLEL_PEEK()));
    KLEL_CAPTURE_PRODUCTION(OpenSquare, KLEL_EXPECT(KLEL_NODE_OPEN_SQUARE));
    KLEL_CAPTURE_PRODUCTION(Codes, KLEL_EXPECT(KLEL_NODE_INTEGER));
    while (KLEL_PEEK() == KLEL_NODE_COMMA)
    {
      KLEL_CAPTURE_PRODUCTION(Comma, KLEL_EXPECT(KLEL_NODE_COMMA));
      KLEL_CAPTURE_PRODUCTION(Code, KLEL_EXPECT(KLEL_NODE_INTEGER));
      KLEL_APPEND_PRODUCTION(Codes, 0, Code);
    }
    KLEL_CAPTURE_PRODUCTION(CloseSquare, KLEL_EXPECT(KLEL_NODE_CLOSE_SQUARE));

    KLEL_ADD_TO_RETURNED_NODE(iCodesIndex, Codes);
  }

  KLEL_ADD_TO_RETURNED_NODE(KLEL_PREDICATE_INDEX,   Predicate);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_INTERPRETER_INDEX, Interpreter);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_COMMAND_INDEX,     Command);
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * Expression <-- Conditional
 *              | Let
 *              | SimpleExpression [Relation SimpleExpression]
 *
 * Relation <-- '<' | '>' | '<=' | '>=' | '!=' | '==' | '=~' | '!~'
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelExpression)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Conditional)
    KLEL_TOKEN_DEF(Let)
    KLEL_TOKEN_DEF(Operator)
    KLEL_TOKEN_DEF(PotentialConditional)
    KLEL_TOKEN_DEF(Relation)
    KLEL_TOKEN_DEF(SimpleExpression)
    KLEL_TOKEN_DEF(SimpleExpressionLeft)
  KLEL_END_PRODUCTION_TOKENS

  int bContinue = 1;

  KLEL_TRY_CAPTURE_PRODUCTION(PotentialConditional, KLEL_INVOKE(KlelPotentialConditional));
  if (KLEL_CAPTURED_PRODUCTION(PotentialConditional) != NULL)
  {
    KLEL_CAPTURE_PRODUCTION(Conditional, KLEL_INVOKE(KlelConditional));
    KLEL_REPLACE_RETURNED_NODE(Conditional);
  }
  else if (KLEL_PEEK() == KLEL_NODE_LET)
  {
    KLEL_CAPTURE_PRODUCTION(Let, KLEL_INVOKE(KlelLet));
    KLEL_REPLACE_RETURNED_NODE(Let);
  }
  else
  {
    KLEL_CAPTURE_PRODUCTION(SimpleExpressionLeft, KLEL_INVOKE(KlelSimpleExpression));
    while (bContinue)
    {
      switch (KLEL_PEEK())
      {
        case KLEL_NODE_EQ_EQ:
        case KLEL_NODE_GT:
        case KLEL_NODE_GTE:
        case KLEL_NODE_LIKE:
        case KLEL_NODE_LT:
        case KLEL_NODE_LTE:
        case KLEL_NODE_NE:
        case KLEL_NODE_UNLIKE:
          KLEL_CAPTURE_PRODUCTION(Operator, KLEL_EXPECT(KLEL_PEEK()));
          KLEL_ROTATE_PRODUCTION(SimpleExpressionLeft, KLEL_OPERAND1_INDEX, Operator);
          KLEL_CAPTURE_PRODUCTION(SimpleExpression, KLEL_INVOKE(KlelSimpleExpression));
          KLEL_APPEND_PRODUCTION(SimpleExpressionLeft, KLEL_OPERAND2_INDEX, SimpleExpression);
          break;

        default:
          bContinue = 0;
          break;
      }
    }
    KLEL_REPLACE_RETURNED_NODE(SimpleExpressionLeft);
  }
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * PotentialConditional <-- Factor '?'
 *
 * Note that this isn't a real part of the grammar. It exists to give
 * KLEL_TRY_CAPTURE_PRODUCTION something to look for to see if we're
 * probably trying to parse a conditional. This lets us generate better
 * error messages.
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelPotentialConditional)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Factor)
    KLEL_TOKEN_DEF(Question)
  KLEL_END_PRODUCTION_TOKENS

  KLEL_CAPTURE_PRODUCTION(Factor,   KLEL_INVOKE(KlelFactor));
  KLEL_CAPTURE_PRODUCTION(Question, KLEL_EXPECT(KLEL_NODE_QUESTION));
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * Conditional <-- Factor '?' Expression ':' Expression
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelConditional)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Colon)
    KLEL_TOKEN_DEF(IfFalse)
    KLEL_TOKEN_DEF(IfTrue)
    KLEL_TOKEN_DEF(Predicate)
    KLEL_TOKEN_DEF(Question)
  KLEL_END_PRODUCTION_TOKENS

  KLEL_CAPTURE_PRODUCTION(Predicate, KLEL_INVOKE(KlelFactor));
  KLEL_CAPTURE_PRODUCTION(Question,  KLEL_EXPECT(KLEL_NODE_QUESTION));
  KLEL_CAPTURE_PRODUCTION(IfTrue,    KLEL_INVOKE(KlelExpression));
  KLEL_CAPTURE_PRODUCTION(Colon,     KLEL_EXPECT(KLEL_NODE_COLON));
  KLEL_CAPTURE_PRODUCTION(IfFalse,   KLEL_INVOKE(KlelExpression));

  KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_CONDITIONAL);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_PREDICATE_INDEX, Predicate);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_IFTRUE_INDEX,    IfTrue);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_IFFALSE_INDEX,   IfFalse);
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * Factor <-- Integer | Real | String | Call | Designator
 *          | '(' Expression ')' | '!' Factor | '~' Factor | '-' Factor
 *
 * Call <-- Designator '(' [Expression {',' Expression}] ')'
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelFactor)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Argument)
    KLEL_TOKEN_DEF(Bang)
    KLEL_TOKEN_DEF(CloseParen)
    KLEL_TOKEN_DEF(Comma)
    KLEL_TOKEN_DEF(Concat)
    KLEL_TOKEN_DEF(Designator)
    KLEL_TOKEN_DEF(Expression)
    KLEL_TOKEN_DEF(Fragment)
    KLEL_TOKEN_DEF(Integer)
    KLEL_TOKEN_DEF(Minus)
    KLEL_TOKEN_DEF(OpenParen)
    KLEL_TOKEN_DEF(Operand)
    KLEL_TOKEN_DEF(Real)
    KLEL_TOKEN_DEF(String)
    KLEL_TOKEN_DEF(Tilde)
  KLEL_END_PRODUCTION_TOKENS

  int iArgumentCount = 0;

  switch (KLEL_PEEK())
  {
    case KLEL_NODE_INTEGER:
      KLEL_CAPTURE_PRODUCTION(Integer, KLEL_EXPECT(KLEL_NODE_INTEGER));
      KLEL_REPLACE_RETURNED_NODE(Integer);
      break;

    case KLEL_NODE_REAL:
      KLEL_CAPTURE_PRODUCTION(Real, KLEL_EXPECT(KLEL_NODE_REAL));
      KLEL_REPLACE_RETURNED_NODE(Real);
      break;

    case KLEL_NODE_QUOTE:
      KLEL_CAPTURE_PRODUCTION(String, KLEL_INVOKE(KlelString));
      KLEL_REPLACE_RETURNED_NODE(String);
      break;

    case KLEL_NODE_DESIGNATOR:
      KLEL_CAPTURE_PRODUCTION(Designator, KLEL_EXPECT(KLEL_NODE_DESIGNATOR));
      KLEL_REPLACE_RETURNED_NODE(Designator);
      KLEL_CAPTURED_PRODUCTION(Designator)->iClosure = KlelClosureForDesignator(psContext, KLEL_CAPTURED_PRODUCTION(Designator)->acFragment);

      if (KLEL_PEEK() == KLEL_NODE_OPEN_PAREN)
      {
        KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_CALL);
        KLEL_CAPTURE_PRODUCTION(OpenParen, KLEL_EXPECT(KLEL_NODE_OPEN_PAREN));
        if (KLEL_PEEK() != KLEL_NODE_CLOSE_PAREN)
        {
          KLEL_CAPTURE_PRODUCTION(Argument, KLEL_INVOKE(KlelExpression));
          KLEL_ADD_TO_RETURNED_NODE(iArgumentCount, Argument);
          iArgumentCount++;

          while (KLEL_PEEK() == KLEL_NODE_COMMA)
          {
            if (iArgumentCount >= KLEL_MAX_FUNC_ARGS)
            {
              KLEL_REPORT_ERROR("too many arguments to function");
            }
            KLEL_CAPTURE_PRODUCTION(Comma, KLEL_EXPECT(KLEL_NODE_COMMA));
            KLEL_CAPTURE_PRODUCTION(Argument, KLEL_INVOKE(KlelExpression));
            KLEL_ADD_TO_RETURNED_NODE(iArgumentCount, Argument);
            iArgumentCount++;
          }
        }
        KLEL_CAPTURE_PRODUCTION(CloseParen, KLEL_EXPECT(KLEL_NODE_CLOSE_PAREN));
      }
      break;

    case KLEL_NODE_OPEN_PAREN:
      KLEL_CAPTURE_PRODUCTION(OpenParen, KLEL_EXPECT(KLEL_NODE_OPEN_PAREN));
      KLEL_CAPTURE_PRODUCTION(Expression, KLEL_INVOKE(KlelExpression));
      KLEL_CAPTURE_PRODUCTION(CloseParen, KLEL_EXPECT(KLEL_NODE_CLOSE_PAREN));
      KLEL_REPLACE_RETURNED_NODE(Expression);
      break;

    case KLEL_NODE_BANG:
      KLEL_CAPTURE_PRODUCTION(Bang, KLEL_EXPECT(KLEL_NODE_BANG));
      KLEL_CAPTURE_PRODUCTION(Operand, KLEL_INVOKE(KlelFactor));
      KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_BANG);
      KLEL_ADD_TO_RETURNED_NODE(KLEL_OPERAND1_INDEX, Operand);
      break;

    case KLEL_NODE_TILDE:
      KLEL_CAPTURE_PRODUCTION(Tilde, KLEL_EXPECT(KLEL_NODE_TILDE));
      KLEL_CAPTURE_PRODUCTION(Operand, KLEL_INVOKE(KlelFactor));
      KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_TILDE);
      KLEL_ADD_TO_RETURNED_NODE(KLEL_OPERAND1_INDEX, Operand);
      break;

    case KLEL_NODE_MINUS:
      KLEL_CAPTURE_PRODUCTION(Minus, KLEL_EXPECT(KLEL_NODE_MINUS));
      KLEL_CAPTURE_PRODUCTION(Operand, KLEL_INVOKE(KlelFactor));
      KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_NEGATE);
      KLEL_ADD_TO_RETURNED_NODE(KLEL_OPERAND1_INDEX, Operand);
      break;

    default:
      KLEL_REPORT_ERROR("expected number, string, call, or expression");
      break;
  }
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * SimpleExpression <-- Term {AddOperator Term}
 *
 * AddOperator <-- '.' | '+' | '-' | '||' | '&' | '^' | '|'
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelSimpleExpression)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Operator)
    KLEL_TOKEN_DEF(TermLeft)
    KLEL_TOKEN_DEF(TermRight)
  KLEL_END_PRODUCTION_TOKENS

  int bContinue = 1;

  KLEL_CAPTURE_PRODUCTION(TermLeft, KLEL_INVOKE(KlelTerm));
  while (bContinue)
  {
    switch (KLEL_PEEK())
    {
      case KLEL_NODE_AND:
      case KLEL_NODE_CARET:
      case KLEL_NODE_DOT:
      case KLEL_NODE_MINUS:
      case KLEL_NODE_PIPE:
      case KLEL_NODE_PIPE_PIPE:
      case KLEL_NODE_PLUS:
        KLEL_CAPTURE_PRODUCTION(Operator, KLEL_EXPECT(KLEL_PEEK()));
        KLEL_CAPTURE_PRODUCTION(TermRight, KLEL_INVOKE(KlelTerm));
        KLEL_ROTATE_PRODUCTION(TermLeft, KLEL_OPERAND1_INDEX, Operator);
        KLEL_APPEND_PRODUCTION(TermLeft, KLEL_OPERAND2_INDEX, TermRight);
        break;

      default:
        bContinue = 0;
        break;
    }
  }
  KLEL_REPLACE_RETURNED_NODE(TermLeft);
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * Term <-- Factor {MulOperator Factor}
 *
 * MulOperator <-- '*' | '/' | '%' | '&&' | '<<' | '>>'
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelTerm)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Operator)
    KLEL_TOKEN_DEF(FactorLeft)
    KLEL_TOKEN_DEF(FactorRight)
  KLEL_END_PRODUCTION_TOKENS

  int bContinue = 1;

  KLEL_CAPTURE_PRODUCTION(FactorLeft, KLEL_INVOKE(KlelFactor));
  while (bContinue)
  {
    switch (KLEL_PEEK())
    {
      case KLEL_NODE_AND_AND:
      case KLEL_NODE_GT_GT:
      case KLEL_NODE_LT_LT:
      case KLEL_NODE_PERCENT:
      case KLEL_NODE_SLASH:
      case KLEL_NODE_STAR:
        KLEL_CAPTURE_PRODUCTION(Operator, KLEL_EXPECT(KLEL_PEEK()));
        KLEL_CAPTURE_PRODUCTION(FactorRight, KLEL_INVOKE(KlelFactor));
        KLEL_ROTATE_PRODUCTION(FactorLeft, KLEL_OPERAND1_INDEX, Operator);
        KLEL_APPEND_PRODUCTION(FactorLeft, KLEL_OPERAND2_INDEX, FactorRight);
        break;

      default:
        bContinue = 0;
        break;
    }
  }
  KLEL_REPLACE_RETURNED_NODE(FactorLeft);
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * Let <-- 'let' Designator '=' Expression 'in' Expression
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelLet)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Definition)
    KLEL_TOKEN_DEF(Designator)
    KLEL_TOKEN_DEF(Equals)
    KLEL_TOKEN_DEF(Expression)
    KLEL_TOKEN_DEF(In)
    KLEL_TOKEN_DEF(Let)
  KLEL_END_PRODUCTION_TOKENS

  KLEL_CAPTURE_PRODUCTION(Let,        KLEL_EXPECT(KLEL_NODE_LET));
  KLEL_CAPTURE_PRODUCTION(Designator, KLEL_EXPECT(KLEL_NODE_DESIGNATOR));
  KLEL_CAPTURE_PRODUCTION(Equals,     KLEL_EXPECT(KLEL_NODE_EQ));
  KLEL_CAPTURE_PRODUCTION(Definition, KLEL_INVOKE(KlelExpression));
  KLEL_CAPTURE_PRODUCTION(In,         KLEL_EXPECT(KLEL_NODE_IN));

  KLEL_PUSH_ENVIRONMENT(psContext, KLEL_CAPTURED_PRODUCTION(Designator)->acFragment);
    KLEL_CAPTURE_PRODUCTION(Expression, KLEL_INVOKE(KlelExpression));
    KLEL_SET_RETURNED_NODE_ENVIRONMENT(sClosure.iIndex);
  KLEL_POP_ENVIRONMENT(psContext);

  KLEL_SET_RETURNED_NODE_TYPE(KLEL_NODE_LET);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_DESIGNATOR_INDEX, Designator);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_DEFINITION_INDEX, Definition);
  KLEL_ADD_TO_RETURNED_NODE(KLEL_EXPRESSION_INDEX, Expression);
KLEL_END_PARSER_PRODUCTION


/*-
 ***********************************************************************
 *
 * KlelString
 *
 ***********************************************************************
 */
KLEL_PARSER_PRODUCTION(KlelString)
  KLEL_PRODUCTION_TOKENS
    KLEL_TOKEN_DEF(Concat)
    KLEL_TOKEN_DEF(CloseQuote)
    KLEL_TOKEN_DEF(FragmentLeft)
    KLEL_TOKEN_DEF(FragmentRight)
    KLEL_TOKEN_DEF(OpenQuote)
  KLEL_END_PRODUCTION_TOKENS

  KLEL_CAPTURE_PRODUCTION(OpenQuote, KLEL_EXPECT(KLEL_NODE_QUOTE));

  if (KLEL_PEEK() == KLEL_NODE_QUOTE)
  {
    KLEL_CAPTURE_PRODUCTION(FragmentLeft, KLEL_CREATE_NODE(KLEL_NODE_FRAGMENT));
  }
  else if (KLEL_PEEK() == KLEL_NODE_FRAGMENT || KLEL_PEEK() == KLEL_NODE_INTERP || KLEL_PEEK() == KLEL_NODE_QUOTED_INTERP)
  {
    KLEL_CAPTURE_PRODUCTION(FragmentLeft, KLEL_EXPECT(KLEL_PEEK()));
    if (KLEL_CAPTURED_PRODUCTION(FragmentLeft)->iType == KLEL_NODE_INTERP || KLEL_CAPTURED_PRODUCTION(FragmentLeft)->iType == KLEL_NODE_QUOTED_INTERP)
    {
      KLEL_CAPTURED_PRODUCTION(FragmentLeft)->iClosure = KlelClosureForDesignator(psContext, KLEL_CAPTURED_PRODUCTION(FragmentLeft)->acFragment);
    }
  }

  while (KLEL_PEEK() == KLEL_NODE_FRAGMENT || KLEL_PEEK() == KLEL_NODE_INTERP || KLEL_PEEK() == KLEL_NODE_QUOTED_INTERP)
  {
    KLEL_CAPTURE_PRODUCTION(FragmentRight, KLEL_EXPECT(KLEL_PEEK()));
    if (KLEL_CAPTURED_PRODUCTION(FragmentRight)->iType == KLEL_NODE_INTERP || KLEL_CAPTURED_PRODUCTION(FragmentRight)->iType == KLEL_NODE_QUOTED_INTERP)
    {
      KLEL_CAPTURED_PRODUCTION(FragmentRight)->iClosure = KlelClosureForDesignator(psContext, KLEL_CAPTURED_PRODUCTION(FragmentRight)->acFragment);
    }
    KLEL_CAPTURE_PRODUCTION(Concat, KLEL_CREATE_NODE(KLEL_NODE_DOT));
    KLEL_ROTATE_PRODUCTION(FragmentLeft, KLEL_OPERAND1_INDEX, Concat);
    KLEL_APPEND_PRODUCTION(FragmentLeft, KLEL_OPERAND2_INDEX, FragmentRight);
  }

  KLEL_CAPTURE_PRODUCTION(CloseQuote, KLEL_EXPECT(KLEL_NODE_QUOTE));
  KLEL_REPLACE_RETURNED_NODE(FragmentLeft);
KLEL_END_PARSER_PRODUCTION
