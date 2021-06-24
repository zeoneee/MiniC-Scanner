# MiniC-Scanner
MiniC-Scanner 확장 구현 
- 기존의 MiniC 언어를 위한 Scanner를 분석하고 확장하는 것을 목표로 함<br>

- 확장 내용
 1. 추가 키워드: char, double, for, do, goto, switch, case, break, default
 2. 추가 연산자: ':'
 3. 추가 인식 리터럴<br>
  (1) character literal<br>
  (2) string literal<br>
  (3) double literal (  .123,   123. 과 같은 숏폼 인식)<br>
 4. 주석<br>
  (1) documented(/** ~ */) comments <br>
  (2) single line documented(///) comments<br>
 5. 추가 토큰 속성값 출력: file name, line number, column number
