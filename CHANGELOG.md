## 0.2.0 (2025-03-25)

### Feat

- **tests**: adds tests to test ability to compute tasks in parallel.
- **TaskWeaver**: adds when_all utility function to be able resolve several futures at once.
- **TaskWeaver**: adds when_all function to be able to wait several futures at once.
- **TaskWeaver**: adds metrix (meta programming lib) as a dependency.
- **TaskWeaver**: adds task managment.

### Fix

- **TaskWeaver**: fixes an issue on attempt to access executor out of the executor array range.
- **TaskWeaver**: fixes an issue when main thread has no executor

## 0.1.0 (2025-03-20)

### Feat

- **tests**: adds tests for task stealing deque.

### Fix

- **TaskWeaver**: fixes item count computation in case of bottom is overflowed.
- **TaskWeaver**: fixes an issue on attempt to pop invalid data.
- **TaskWeaver**: fixes an issue with wrong IsEmpty condition
