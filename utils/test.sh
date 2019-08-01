#!/bin/sh

PROGRAM=`basename ${0}`

case "${OS}" in
Windows_*)
  export PRINTF_EXPONENT_DIGITS=2 # This is needed for several real tests.
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
  INITIAL_OUTPUT="`${KLEL_EXPR} -b 1 -e \"${1}\" 2> /dev/null`"
  MY_OBSERVED_STATUS=$?
  MY_OBSERVED_OUTPUT="`echo \"${INITIAL_OUTPUT}\" | sed '1,/EndOfOutput$/d; /^EndOfOutput/d;'`"
  MY_EXEC_TIME="`echo \"${INITIAL_OUTPUT}\" | egrep '^KlelExecTotTime=' | awk -F= '{print $2}' | sed 's/^.//; s/.$//;'`"
  if [ -z "${MY_EXEC_TIME}" ] ; then
    MY_EXEC_TIME="X.XXXXXX"
  fi
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
  echo "Expected='${MY_EXPECTED_RESULT}'; Observed='${MY_OBSERVED_RESULT}'; Result='${MY_TEST_RESULT}'; Status='${MY_OBSERVED_STATUS}'; Time='${MY_EXEC_TIME}'; Output='${MY_OBSERVED_OUTPUT}'; Expression='${1}';"
}

#######################################################################
#
# Pass
#
#######################################################################

Pass()
{
  INITIAL_OUTPUT="`${KLEL_EXPR} -b 1 -e \"${1}\" 2> /dev/null`"
  MY_OBSERVED_STATUS=$?
  MY_OBSERVED_OUTPUT="`echo \"${INITIAL_OUTPUT}\" | sed '1,/EndOfOutput$/d; /^EndOfOutput/d;'`"
  MY_EXEC_TIME="`echo \"${INITIAL_OUTPUT}\" | egrep '^KlelExecTotTime=' | awk -F= '{print $2}' | sed 's/^.//; s/.$//;'`"
  if [ -z "${MY_EXEC_TIME}" ] ; then
    MY_EXEC_TIME="X.XXXXXX"
  fi
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
  echo "Expected='${MY_EXPECTED_RESULT}'; Observed='${MY_OBSERVED_RESULT}'; Result='${MY_TEST_RESULT}'; Status='${MY_OBSERVED_STATUS}'; Time='${MY_EXEC_TIME}'; Output='${MY_OBSERVED_OUTPUT}'; Expression='${1}';"
}

#######################################################################
#
# PassExitStatus
#
#######################################################################

PassExitStatus()
{
  INITIAL_OUTPUT="`${KLEL_EXPR} -b 1 -d -e \"${1}\" 2> /dev/null`"
  MY_OBSERVED_STATUS=$?
  MY_OBSERVED_OUTPUT="`echo \"${INITIAL_OUTPUT}\" | egrep '^KlelCommandStatus=' | awk -F= '{print $2}' | sed 's/^.//; s/.$//;'`"
  MY_EXEC_TIME="`echo \"${INITIAL_OUTPUT}\" | egrep '^KlelExecTotTime=' | awk -F= '{print $2}' | sed 's/^.//; s/.$//;'`"
  if [ -z "${MY_EXEC_TIME}" ] ; then
    MY_EXEC_TIME="X.XXXXXX"
  fi
  MY_EXPECTED_RESULT=${2}
  MY_OBSERVED_RESULT=${MY_OBSERVED_OUTPUT}
  if [ ${MY_OBSERVED_STATUS} -eq 0 -a X"${MY_OBSERVED_RESULT}" = X"${MY_EXPECTED_RESULT}" ] ; then
    MY_TEST_RESULT="pass"
  else
    MY_TEST_RESULT="fail"
    ERRORS="1"
  fi
  echo "Expected='${MY_EXPECTED_RESULT}'; Observed='${MY_OBSERVED_RESULT}'; Result='${MY_TEST_RESULT}'; Status='${MY_OBSERVED_STATUS}'; Time='${MY_EXEC_TIME}'; Output='${MY_OBSERVED_OUTPUT}'; Expression='${1}';"
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

  Pass  '1 + 2'           '3'
  Pass  '2 - 1'           '1'
  Pass  'true || false'   'true'
  Pass  '0xff & 0x01'     '1'
  Pass  '0xff ^ 0x01'     '254'
  Pass  '0xff | 0x01'     '255'
  Fail  '0xff + "hello"'

  #####################################################################
  #
  # Character =  '\' ('\' | '"' | 'n' | 'r' | 'x' hexdig hexdig | '$')
  # Character =/ '${' designator '}'
  # Character =/ <any character other than '\', '"', 'n', 'r', '$' or
  #               control>
  #
  #####################################################################

  #Pass '"hello\\\"'       'hello\\'
  Pass  '"hello\""'       'hello"'
  Pass  '"hello\x41"'     'helloA'
  Pass  '"hello%{true}"'  'hellotrue'
  Fail  '"hello%{unknown}'
  Fail  '"hello%{imcomplete"'

  #####################################################################
  #
  # Conditional = Factor '?' Expression ':' Expression
  #
  #####################################################################

  Pass  'true ? 1 : 2'    '1'
  Pass  'false ? 1 : 2'   '2'
  Fail  '"hello" ? 1 : 2'

  #####################################################################
  #
  # Call = Designator '(' [ExprList] ')'
  #
  #####################################################################

  Pass  'entier(2.0)'     '2'
  Pass  'max(1,2)'        '2'
  Pass  'min(1,2)'        '1'
  Fail  'entier(2)'
  Fail  'strlen(false)'

  #####################################################################
  #
  # Factor =   Number | String | Call | Designator
  # Factor =/  '(' Expression ')' | '!' Factor | '-' Factor
  # Factor =/  '~' Factor
  #
  #####################################################################

  Pass  '(true && true) ? 1 : 2'    '1'
  Pass  '(true && false) ? 1 : 2'   '2'
  Pass  '!true ? 1 : 2'             '2'
  Pass  '!false ? 1 : 2'            '1'
  Pass  '-3'                        '-3'
  Pass  '3 - -3'                    '6'
  Pass  '3 --3'                     '6'
  Pass  '3--3'                      '6'

  #####################################################################
  #
  # GuardedCommand =   '{' Expression '}' '->' '[' ExprList ']'
  # GuardedCommand =/  '{' Expression '}' '->' String
  #
  #####################################################################

  Pass  'if (true && true) then eval("echo", "hello")'             'hello'
  Pass  'if (true && true) then eval("echo", "ls", "-l", true)'    'ls -l true'
  Pass  'if (true && false) then eval("echo", "hello")'            ''
  Fail  'if (entier(2.0)) then eval("echo", "hello")'
  Fail  'if (entier(2.0)) then 2'
  Fail  'if (true) then eval("echo", "", (1 / 0))'
  Fail  'if (true) then eval("echo", "", (1.0 / 0.0))'
  Fail  'if (true) then eval("echo", "", abort("self-induced failure"))'
  Fail  'if (true) then eval("system", "", (1 / 0))'
  Fail  'if (true) then eval("system", "", (1.0 / 0.0))'
  Fail  'if (true) then eval("system", "", abort("self-induced failure"))'

  #####################################################################
  #
  # MulOperator = '*' | '/' | '%' | '&&' | '<<' | '>>'
  #
  #####################################################################

  Pass  '1 * 0'       '0'
  Pass  '2 * 3'       '6'
  Pass  '1 + 2 * 3'   '7'
  Pass  '6 / 2'       '3'
  Pass  '2 << 1'      '4'
  Pass  '8 >> 2'      '2'
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

  Pass  '0xff'    '255'
  Pass  '0o101'   '65'
  Pass  '2.0'     '2'
  Pass  '2.1'     '2.1'
  Pass  '2.1e-77' '2.1e-77'
  Pass  '2.1e+77' '2.1e+77'
  Pass  '2.1e77'  '2.1e+77'
  Fail  '2.1e'
  Fail  '2.'
  Fail  '0xgg'
  Fail  '0o9'

  #####################################################################
  #
  # Relation = '<' | '>' | '<=' | '>=' | '!=' | '==' | '=~' | '!~'
  #
  #####################################################################

  Pass  '1 < 2'          'true'
  Pass  '2 > 1'          'true'
  Pass  '1 <= 7'         'true'
  Pass  '7 >= 10'        'false'
  Pass  '8 != 9'         'true'
  Pass  '"h" != "h"'     'false'
  Pass  '8 == 8'         'true'
  Pass  '"h" == "g"'     'false'
  Pass  'true == true'   'true'
  Pass  '2.0 == 2.0'     'true'
  Pass  '"h" =~ "[a-z]"' 'true'
  Pass  '"h" !~ "[a-z]"' 'false'
  Fail  '"h" == 1"'
  Fail  '"h" =~ 1'
  Fail  '1 !~ 1'

  #####################################################################
  #
  # Let Expressions
  #
  #####################################################################

  Pass 'let a = 1 in a'                                   '1'
  Pass 'let a = 1 in let b = 2 in a + b'                  '3'
  Pass 'let a = 1 in let b = 2 in let c = 3 in a + b + c' '6'
  Pass 'let a = 1 in let b = a + 1 in b + 2'              '4'
  Pass 'let a = "world" in "hello, %{a}!"'                'hello, world!'
  Fail 'let a = a + 1 in a'
  Fail 'let a ='


  #####################################################################
  #
  # Standard library tests.
  #
  #####################################################################

  Pass  'true  == (min(1,2) == 1)'   'true'
  Pass  'false == (min(1,2) == 2)'   'true'
  Pass  'entier(pi) == 3'            'true'
  Pass  'entier(e) == 2'             'true'
  Pass  'abs(-1)'                    '1'
  Pass  'abs(1)'                     '1'
  Pass  'real(2) == 2.0'             'true'
  Pass  'max(1,2)'                   '2'
  Pass  'min(1,2)'                   '1'
  Pass  'odd(3)'                     'true'
  Pass  'odd(2)'                     'false'
  Pass  'cksum("hello")'             '8403'
  Pass  'chr(0x41)'                  'A'
  Pass  'ord("A")'                   '65'
  Pass  'strlen("hello")'            '5'
  Pass  'strlen("")'                 '0'
  Pass  'string_of_bool(true)'       'true'
  Pass  'string_of_real(2.1)'        '2.1'
  Pass  'string_of_int(2)'           '2'
  Pass  'dec_of_int(512)'            '512'
  Pass  'hex_of_int(512)'            '200'
  Pass  'oct_of_int(512)'            '1000'
  Fail  'abort("fail")'
  Fail  'abort_bool("fail")'
  Fail  'abort_int("fail")'
  Fail  'abort_real("fail")'
  Fail  'abort_string("fail")'
  Fail  '1.0 / 0.0'
  Fail  '1 / 0'
  Fail  'string_of_int(max(1 / 0, 2))'
  Fail  'abs(?)'
  Fail  'abs(2.0)'
  Fail  'max(?)'
  Fail  'max(1)'
  Fail  'min(?)'
  Fail  'min(1)'
  Fail  'odd(?)'
  Fail  'entier(?)'
  Fail  'entier(2)'
  Fail  'real(?)'
  Fail  'unknown'

  #####################################################################
  #
  # String concatenation and interpolation tests
  #
  #####################################################################

  Pass 'let i = now() in strlen(string_of_int(i) . string_of_int(i) . "%{i}%(i)")' '40'
  Pass 'let i = now() in let j = string_of_int(now()) in strlen(string_of_int(i) . (j) . "%{i}%(j)")' '40'
  Pass 'strlen("" . "a" . "aa" . "aaa" . "aaaa" . "aaaaa" . "aaaaaa" . "aaaaaaa" . "aaaaaaaa" .  "aaaaaaaaa" . "aaaaaaaaaa" .  "aaaaaaaaaaa" .  "aaaaaaaaaaaa" .  "aaaaaaaaaaaaa" .  "aaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaaaaaaa" .  "aaaaaaaaaaaaaaaaaaaaaaaa")' '300'
  Pass 'let i = 10 in "%{i}%{true}%(i)"' '10true10'
  Pass 'let i = "&" in "%{i}%(i)%{i}"' '&\&&'

  #####################################################################
  #
  # Guarded command exit status tests.
  #
  #####################################################################

  PassExitStatus  'if (true) then eval("system", "true") pass [0]'       'pass'
  PassExitStatus  'if (true) then eval("system", "true") pass [1]'       'fail'
  PassExitStatus  'if (true) then eval("system", "false") pass [0]'      'fail'
  PassExitStatus  'if (true) then eval("system", "false") pass [1,255]'  'pass'

  #####################################################################
  #
  # If any test fails, the exit status must be nonzero.
  #
  #####################################################################

  exit ${ERRORS}

