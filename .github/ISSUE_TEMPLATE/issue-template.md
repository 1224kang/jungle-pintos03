---
name: issue template
about: PintOS issue template
title: ''
labels: ''
assignees: ''

---

## 관련 이슈
- close #이슈번호

## 작업 내용
<!-- 구현한 기능을 간략히 설명해주세요 (예: Alarm Clock, Priority Scheduling, System Call 등) -->
- 

## 변경 사항
<!-- 수정한 파일과 함수, 추가한 자료구조/필드를 구체적으로 적어주세요 -->
| 파일 | 변경 내용 |
| --- | --- |

## 동기화 / 설계 포인트
<!-- 인터럽트 비활성화, lock/semaphore 사용 위치, race condition 고려 사항 등을 적어주세요 -->
- 

## 체크리스트
- [ ] 코드 컨벤션(GNU 스타일, 들여쓰기 등)을 지켰나요?
- [ ] 이전 프로젝트 테스트가 깨지지 않았나요? (regression)
- [ ] 디버깅용 `printf` 등 불필요한 코드를 제거했나요?

## 🧪 테스트 결과
<!-- 통과/실패한 테스트와 결과를 적어주세요 -->
- 통과: `N / M`
- 통과한 테스트:
- 실패한 테스트:

```bash
# 실행한 명령어
make check
pintos -- -q run alarm-multiple
```

## 🧪 테스트 방법
<!-- 리뷰어가 어떻게 확인하면 되는지 적어주세요 -->
1. `cd threads && make`
2. `cd build && make check`

## 💬 리뷰 요구사항 (선택)
<!-- 설계상 고민했던 부분, 확신이 없는 동기화 처리 등을 적어주세요 -->
- 

## 📝 참고 사항
<!-- 남은 이슈, 다음 단계에서 이어서 할 작업, 참고한 자료(GitBook 등) -->
-
