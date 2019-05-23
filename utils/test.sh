#!/bin/sh

PROGRAM=`basename ${0}`

case "${OS}" in
Windows_*)
  export PRINTF_EXPONENT_DIGITS=2 # This is needed for several float tests.
  ;;
esac

KLEL_EXPR=${1-src/klel-expr/klel-expr}

if [ ! -x "${KLEL_EXPR}" ] ; then
  echo "${PROGRAM}: Error='The target program (${KLEL_EXPR}) does not exist or is not executable.'" 1>&2
  exit 1
fi

ERRORS="0"

#######################################################################
#
# Fail
#
#######################################################################

Fail()
{
  INITIAL_OUTPUT="`${KLEL_EXPR} \"${1}\" 2> /dev/null`"
  MY_OBSERVED_STATUS=$?
  MY_OBSERVED_OUTPUT="`echo \"${INITIAL_OUTPUT}\" | cut -d ':' -f 2,3 | sed 's/^ //'`"
  if [ ${MY_OBSERVED_STATUS} -eq 0 ] ; then
    MY_EXPECTED_RESULT="fail"
    MY_OBSERVED_RESULT="pass"
    MY_TEST_RESULT="fail"
    ERRORS="1"
  else
    MY_EXPECTED_RESULT="fail"
    MY_OBSERVED_RESULT="fail"
    MY_TEST_RESULT="pass"
  fi
  echo "Expected='${MY_EXPECTED_RESULT}'; Observed='${MY_OBSERVED_RESULT}'; Result='${MY_TEST_RESULT}'; Status='${MY_OBSERVED_STATUS}'; Output='${MY_OBSERVED_OUTPUT}'; Expression='${1}';"
}

#######################################################################
#
# Pass
#
#######################################################################

Pass()
{
  INITIAL_OUTPUT="`${KLEL_EXPR} \"${1}\" 2> /dev/null`"
  MY_OBSERVED_STATUS=$?
  MY_OBSERVED_OUTPUT="`echo \"${INITIAL_OUTPUT}\" | cut -d ':' -f 2,3 | sed 's/^ //'`"
  if [ ${MY_OBSERVED_STATUS} -eq 0 -a X"${MY_OBSERVED_OUTPUT}" = X"${2}" ] ; then
    MY_EXPECTED_RESULT="pass"
    MY_OBSERVED_RESULT="pass"
    MY_TEST_RESULT="pass"
  else
    MY_EXPECTED_RESULT="pass"
    MY_OBSERVED_RESULT="fail"
    MY_TEST_RESULT="fail"
    ERRORS="1"
  fi
  echo "Expected='${MY_EXPECTED_RESULT}'; Observed='${MY_OBSERVED_RESULT}'; Result='${MY_TEST_RESULT}'; Status='${MY_OBSERVED_STATUS}'; Output='${MY_OBSERVED_OUTPUT}'; Expression='${1}';"
}

#######################################################################
#
# Parser tests
#
#######################################################################

  #####################################################################
  #
  # AddOperator = '.' | '+' | '-' | '||' | '&' | '^' | '|'
  #
  #####################################################################

  Pass  '1 + 2'           'int : 3'
  Pass  '2 - 1'           'int : 1'
  Pass  'true || false'   'bool : true'
  Pass  '0xff & 0x01'     'int : 1'
  Pass  '0xff ^ 0x01'     'int : 254'
  Pass  '0xff | 0x01'     'int : 255'
  Fail  '0xff + "hello"'

  #####################################################################
  #
  # Character =  '\' ('\' | '"' | 'n' | 'r' | 'x' hexdig hexdig | '$')
  # Character =/ '${' designator '}'
  # Character =/ <any character other than '\', '"', 'n', 'r', '$' or
  #               control>
  #
  #####################################################################

  #Pass '"hello\\\"'       'string : hello\\'
  Pass  '"hello\""'       'string : hello"'
  Pass  '"hello\x41"'     'string : helloA'
  Pass  '"hello%{true}"'  'string : hellotrue'
  Fail  '"hello%{unknown}'
  Fail  '"hello%{imcomplete"'

  #####################################################################
  #
  # Conditional = Factor '?' Expression ':' Expression
  #
  #####################################################################

  Pass  'true ? 1 : 2'    'int : 1'
  Pass  'false ? 1 : 2'   'int : 2'
  Fail  '"hello" ? 1 : 2'

  #####################################################################
  #
  # Call = Designator '(' [ExprList] ')'
  #
  #####################################################################

  Pass  'entier(2.0)'     'int : 2'
  Pass  'max(1,2)'        'int : 2'
  Pass  'min(1,2)'        'int : 1'
  Fail  'entier(2)'
  Fail  'strlen(false)'

  #####################################################################
  #
  # Factor =   Number | String | Call | Designator
  # Factor =/  '(' Expression ')' | '!' Factor | '-' Factor
  # Factor =/  '~' Factor
  #
  #####################################################################

  Pass  '(true && true) ? 1 : 2'    'int : 1'
  Pass  '(true && false) ? 1 : 2'   'int : 2'
  Pass  '!true ? 1 : 2'             'int : 2'
  Pass  '!false ? 1 : 2'            'int : 1'
  Pass  '-3'                        'int : -3'
  Pass  '3 - -3'                    'int : 6'
  Pass  '3 --3'                     'int : 6'
  Pass  '3--3'                      'int : 6'

  #####################################################################
  #
  # GuardedCommand =   '{' Expression '}' '->' '[' ExprList ']'
  # GuardedCommand =/  '{' Expression '}' '->' String
  #
  #####################################################################

  Pass  'if (true && true) then eval("echo", "hello")'             'echo hello'
  Pass  'if (true && true) then eval("echo", "ls", "-l", true)'    'echo ls -l true'
  Pass  'if (true && false) then eval("echo", "hello")'            'bool : false'
  Fail  'if (entier(2.0)) then eval("echo", "hello")'
  Fail  'if (entier(2.0)) then 2'

  #####################################################################
  #
  # MulOperator = '*' | '/' | '%' | '&&' | '<<' | '>>'
  #
  #####################################################################

  Pass  '1 * 0'       'int : 0'
  Pass  '2 * 3'       'int : 6'
  Pass  '1 + 2 * 3'   'int : 7'
  Pass  '6 / 2'       'int : 3'
  Pass  '2 << 1'      'int : 4'
  Pass  '8 >> 2'      'int : 2'
  Fail  '1 / 0'
  Fail  '1 % 0'

  #####################################################################
  #
  # Number =  '0x' hexdigit {hexdigit}
  # Number =/ '0o' {octdigit}
  # Number =/ digit {digit} ['.' digit {digit}]
  #                         ['e' ['+' | '-'] digit {digit}]
  #
  #####################################################################

  Pass  '0xff'    'int : 255'
  Pass  '0o101'   'int : 65'
  Pass  '2.0'     'float : 2'
  Pass  '2.1'     'float : 2.1'
  Pass  '2.1e-77' 'float : 2.1e-77'
  Pass  '2.1e+77' 'float : 2.1e+77'
  Pass  '2.1e77'  'float : 2.1e+77'
  Fail  '0xgg'
  Fail  '0o9'
  Fail  '2.1e'
  Fail  '2.'

  #####################################################################
  #
  # Relation = '<' | '>' | '<=' | '>=' | '!=' | '==' | '=~' | '!~'
  #
  #####################################################################

  Pass  '1 < 2'          'bool : true'
  Pass  '2 > 1'          'bool : true'
  Pass  '1 <= 7'         'bool : true'
  Pass  '7 >= 10'        'bool : false'
  Pass  '8 != 9'         'bool : true'
  Pass  '"h" != "h"'     'bool : false'
  Pass  '8 == 8'         'bool : true'
  Pass  '"h" == "g"'     'bool : false'
  Pass  'true == true'   'bool : true'
  Pass  '2.0 == 2.0'     'bool : true'
  Pass  '"h" =~ "[a-z]"' 'bool : true'
  Pass  '"h" !~ "[a-z]"' 'bool : false'
  Fail  '"h" == 1"'
  Fail  '"h" =~ 1'
  Fail  '1 !~ 1'

  #####################################################################
  #
  # Standard library tests.
  #
  #####################################################################

  Pass  'true  == (min(1,2) == 1)'   'bool : true'
  Pass  'false == (min(1,2) == 2)'   'bool : true'
  Pass  'entier(pi) == 3'            'bool : true'
  Pass  'entier(e) == 2'             'bool : true'
  Pass  'int_bits > 0'               'bool : true'
  Pass  'abs(-1)'                    'int : 1'
  Pass  'abs(1)'                     'int : 1'
  Pass  'float(2) == 2.0'            'bool : true'
  Pass  'max(1,2)'                   'int : 2'
  Pass  'min(1,2)'                   'int : 1'
  Pass  'odd(3)'                     'bool : true'
  Pass  'odd(2)'                     'bool : false'
  Pass  'cksum("hello")'             'int : 8403'
  Pass  'chr(0x41)'                  'string : A'
  Pass  'ord("A")'                   'int : 65'
  Pass  'strlen("hello")'            'int : 5'
  Pass  'strlen("")'                 'int : 0'
  Pass  'string_of_bool(true)'       'string : true'
  Pass  'string_of_float(2.1)'       'string : 2.1'
  Pass  'string_of_int(2)'           'string : 2'
  Fail  'abs(2.0)'
  Fail  'min(1)'
  Fail  'entier(2)'
  Fail  'unknown'

  #####################################################################
  #
  # If any test fails, the exit status must be nonzero.
  #
  #####################################################################

  exit ${ERRORS}

