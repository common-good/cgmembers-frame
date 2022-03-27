Feature: Multiple Choice
AS a member
I WANT to participate in a multiple-choice vote
SO I have a say in decisions that impact me

Setup:
  Given members:
  | uid  | fullName   | email | flags   |*
  | .ZZA | Abe One    | a@    | ok      |
  | .ZZB | Bea Two    | b@    | ok      |
  | .ZZC | Corner Pub | c@    | ok,co   |
  | .ZZD | Dee Four   | d@    | ok      |
  | .ZZE | Eve Five   | e@    | ok      |
  | .ZZF | Flo Six    | f@    | ok      |
  | .ZZG | Guy Seven  | g@    | ok      |
  | .ZZH | Hal Eight  | h@    | ok      |
  | .ZZI | Ida Nine   | i@    | ok      |
  And member ".ZZA" has "vote" steps done: "all"
  And these "r_proxies":
  | person | proxy | priority |*
  | .ZZA   | .ZZB  |        1 |
  | .ZZA   | .ZZD  |        2 |
  | .ZZB   | .ZZA  |        1 |
  | .ZZB   | .ZZE  |        2 |
  | .ZZD   | .ZZA  |        1 |
  | .ZZD   | .ZZF  |        2 |
  | .ZZE   | .ZZD  |        1 |
  | .ZZE   | .ZZB  |        2 |
  | .ZZF   | .ZZD  |        1 |
  | .ZZF   | .ZZE  |        2 |
  | .ZZG   | .ZZH  |        1 |
  | .ZZG   | .ZZI  |        2 |
  | .ZZH   | .ZZI  |        1 |
  | .ZZH   | .ZZG  |        2 |
  | .ZZI   | .ZZG  |        1 |
  | .ZZI   | .ZZF  |        2 |
  And these "r_events":
  | id | ctty | type | event         | start    | end      |*
  |  1 | ctty |    V | Annual Trivia | %now0-1w | %now0+1d |
  |  2 | ctty |    G | Grade         | %now0+2w | %now0+3w |
  And these "r_questions":
  | id | event | text                | type | optOrder | created |*
  | 5  | 1     | What Color is Best? | M    | N        | %now-8d |
  And these "r_options":
  | id | question | text  | created |*
  | 7  | 5        | red   | %now-8d |
  | 8  | 5        | blue  | %now-8d |
  | 9  | 5        | green | %now-8d |

Scenario: A member votes
  When member ".ZZA" visits page "community/events"
  Then we show "Community Democracy" with:
  | Common Good Western Mass ||
  | Status: Annual Trivia from NOW to %mdY+1d ||
  | Vote ||
  | make sure you are happy with your proxy choices ||
  | community comes first | I agree. |
  | Vote Now ||
  | Polls close at midnight on | %mdY+1d |
  | Event History ||
  | Ends    | Event |
  | %mdY+3w | Grade |
  | %mdY+1d | Annual Trivia |
  
  When member ".ZZA" completes form "community/events" with values:
  | op       | agree |*
  | Vote Now |    on |
  Then we show "Community Democracy" with:
  | CGVoting    | (Question #1 of 1) |
  | QUESTION #1 | What Color is Best? |
  And with:
  | Comment | Veto | E | D | C | B | A | red   |
  And with:
  | Comment | Veto | E | D | C | B | A | blue  |
  And with:
  | Comment | Veto | E | D | C | B | A | green |
  And these "r_ballots":
  | id | question | voter | proxy |*
  |  1 |        5 |  .ZZA |     0 |
  And these "r_votes":
  | id | ballot | option | grade | gradeMax | displayOrder | text | isVeto |*
  |  1 |      1 |      7 |    -1 |       -1 |            0 |      |      0 |
  |  2 |      1 |      8 |    -1 |       -1 |            1 |      |      0 |
  |  3 |      1 |      9 |    -1 |       -1 |            2 |      |      0 |
  
  When member ".ZZA" completes form "community/events" with values:
  | op                    | question | ballot | vote0 | vote1 | vote2 | option0 | option1 | option2 | optionCount |*
  | Done With Question #1 | 5        | 1      | 1     | 2     | 3      | 4       | 3       | 2       | 3           |
  Then we show "Community Democracy" with:
  | Thank You for Voting! |
  And these "r_ballots":
  | id | question | voter | proxy |*
  |  1 |        5 |  .ZZA |  .ZZA |
  And these "r_votes":
  | id | ballot | option | grade | gradeMax | displayOrder | text | isVeto |*
  |  1 |      1 |      7 |    12 |       -1 |            0 |      |      0 |
  |  2 |      1 |      8 |     9 |       -1 |            1 |      |      0 |
  |  3 |      1 |      9 |     6 |       -1 |            2 |      |      0 |

Scenario: A member views vote results
  Given these "r_ballots":
  | id | question | voter | proxy |*
  |  1 |        5 |  .ZZA |  .ZZA |
  And these "r_votes":
  | id | ballot | option | grade | gradeMax | displayOrder | text | isVeto |*
  |  1 |      1 |      7 |    12 |       -1 |            0 |      |      0 |
  |  2 |      1 |      8 |     9 |       -1 |            1 |      |      0 |
  |  3 |      1 |      9 |     6 |       -1 |            2 |      |      0 |
  And member ".ZZB" has "vote" steps done: "all"
  And the time now is "%now+2d"
  
  When member ".ZZB" visits "community/events/"
  Then we show "Community Democracy" with:
  | Event History |||
  | Ends    | Event ||
  | %mdY+3w | Grade ||
  | %mdY+1d | Annual Trivia | Results |

  When member ".ZZB" visits "community/events/do=results&eid=1"
  Then we show "Community Democracy" with:
  | Common Good Western Mass ||
  | Question 1 | What Color is Best? |
  | 5 voters   | 1 voted directly, 4 by proxy |
  | 1 | red   |
  | 2 | blue  |
  | 3 | green |
  And with:
  |        | Preferred Over | Average	 | Median   |       |
  And with:
  | Option | 1 | 2 | 3      |   | Vote |   | Vote | Vetos |
  | 1      | - | 5 | 5      | A | 4.00 | A | 4.00	| 0     |
  | 2      | 0 | - | 5      | B | 3.00 | B | 3.00	| 0     |
  | 3      | 0 | 0 | -      | C | 2.00 | C | 2.00	| 0     |
