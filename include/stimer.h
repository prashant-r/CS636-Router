/**
 * A timer.
 *
 * This structure is used for declaring a timer. The timer must be set
 * with stimer_set() before it can be used.
 *
 * \hideinitializer
 */
struct stimer {
  unsigned long start;
  unsigned long interval;
};

void stimer_set(struct stimer *t, unsigned long interval);
void stimer_reset(struct stimer *t);
void stimer_restart(struct stimer *t);
int stimer_expired(struct stimer *t);
unsigned long stimer_remaining(struct stimer *t);
unsigned long stimer_elapsed(struct stimer *t);


#endif /* STIMER_H_ */

/** @} */
/** @} */