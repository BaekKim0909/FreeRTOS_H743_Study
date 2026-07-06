# osMessageQueuePut / osMessageQueueGet 多生产者多消费者任务调度流程

## 系统模型

```
Producer_A (优先级高)  ──┐
Producer_B (优先级低)  ──┼──► [ Message Queue ] ──┬──► Consumer_X (优先级高)
Producer_C (优先级中)  ──┘                        └──► Consumer_Y (优先级低)
```

## osMessageQueuePut 流程（生产者侧）

```mermaid
flowchart TD
    A[Producer 调用 osMessageQueuePut] --> B{队列是否已满?}

    B -->|未满| C[消息写入队列尾部]
    C --> D{是否有 Consumer 阻塞在该队列?}
    D -->|无| E[Put 返回 osOK, Producer 继续运行]
    D -->|有| F[将最高优先级的阻塞 Consumer 从等待列表移到就绪列表]
    F --> G{被唤醒的 Consumer 优先级 > 当前 Producer?}
    G -->|是| H[触发上下文切换, CPU 切到该 Consumer]
    G -->|否| I[Put 返回 osOK, Producer 继续运行]

    B -->|已满| J{timeout 参数?}
    J -->|0 / osNoWait| K[立即返回 osErrorResource]
    J -->|osWaitForever 或 有限时间| L[Producer 进入阻塞态, 挂到队列满等待列表]
    L --> M[调度器选择就绪列表中最高优先级任务运行]
    M --> N{等待期间有 Consumer 取走消息?}
    N -->|是| O[Producer 被唤醒, 消息写入队列, 返回 osOK]
    N -->|超时| P[返回 osErrorTimeout]
```

## osMessageQueueGet 流程（消费者侧）

```mermaid
flowchart TD
    A[Consumer 调用 osMessageQueueGet] --> B{队列是否为空?}

    B -->|非空| C[从队列头部取出消息]
    C --> D{是否有 Producer 阻塞在该队列 - 队列满等待?}
    D -->|无| E[Get 返回 osOK, Consumer 继续运行]
    D -->|有| F[将最高优先级的阻塞 Producer 从等待列表移到就绪列表]
    F --> G[该 Producer 的消息写入队列]
    G --> H{被唤醒的 Producer 优先级 > 当前 Consumer?}
    H -->|是| I[触发上下文切换, CPU 切到该 Producer]
    H -->|否| J[Get 返回 osOK, Consumer 继续运行]

    B -->|空| K{timeout 参数?}
    K -->|0 / osNoWait| L[立即返回 osErrorResource]
    K -->|osWaitForever 或 有限时间| M[Consumer 进入阻塞态, 挂到队列空等待列表]
    M --> N[调度器选择就绪列表中最高优先级任务运行]
    N --> O{等待期间有 Producer 放入消息?}
    O -->|是| P[Consumer 被唤醒, 取出消息, 返回 osOK]
    O -->|超时| Q[返回 osErrorTimeout]
```

## 多生产者多消费者竞争调度示例

```mermaid
sequenceDiagram
    participant PA as Producer_A (Pri=3)
    participant PB as Producer_B (Pri=1)
    participant Q as MessageQueue (depth=2)
    participant CX as Consumer_X (Pri=4)
    participant CY as Consumer_Y (Pri=2)
    participant SCH as Scheduler

    Note over Q: 队列空, Consumer_X 和 Consumer_Y 均阻塞等待消息

    PA->>Q: osMessageQueuePut(msg1)
    Q->>SCH: Consumer_X(Pri=4) 被唤醒
    SCH->>CX: 上下文切换 (Pri=4 > Pri=3)
    CX->>Q: osMessageQueueGet() → 取得 msg1
    Note over CX: 处理完毕后再次 Get, 队列空, 阻塞

    PB->>Q: osMessageQueuePut(msg2)
    Q->>SCH: Consumer_X(Pri=4) 被唤醒
    SCH->>CX: 上下文切换 (Pri=4 > Pri=1)
    CX->>Q: osMessageQueueGet() → 取得 msg2

    Note over Q: 队列满场景
    PA->>Q: osMessageQueuePut(msg3) → 队列满
    PA->>SCH: Producer_A 阻塞
    PB->>Q: osMessageQueuePut(msg4) → 队列满
    PB->>SCH: Producer_B 阻塞

    CY->>Q: osMessageQueueGet() → 取出一条消息
    Q->>SCH: Producer_A(Pri=3) 被唤醒 (优先级最高的生产者优先)
    SCH->>PA: 上下文切换到 Producer_A
    PA->>Q: msg3 写入队列
```

## 关键调度规则总结

| 场景 | 调度行为 |
|------|----------|
| Put 成功且有阻塞的 Consumer | 唤醒**最高优先级**的 Consumer；若其优先级高于当前任务则立即切换 |
| Get 成功且有阻塞的 Producer | 唤醒**最高优先级**的 Producer；若其优先级高于当前任务则立即切换 |
| 多个同优先级任务阻塞 | 按 FIFO 顺序唤醒（先阻塞的先唤醒） |
| Put 时队列满 / Get 时队列空 | 当前任务进入阻塞态，调度器切换到就绪列表最高优先级任务 |
| ISR 中调用 Put | 不会立即切换；在退出 ISR 时由 PendSV 完成上下文切换 |