
#define SCLOCK_GEQ(a, b)	((unsigned long)((a) - (b)) < \
				((unsigned long)(~((unsigned long)0)) >> 1))

/*---------------------------------------------------------------------------*/
/**
 * Set a timer.
 *
 * This function is used to set a timer for a time sometime in the
 * future. The function stimer_expired() will evaluate to true after
 * the timer has expired.
 *
 * \param t A pointer to the timer
 * \param interval The interval before the timer expires.
 *
 */
void
stimer_set(struct stimer *t, unsigned long interval)
{
  t->interval = interval;
  t->start = clock_seconds();
}
/*---------------------------------------------------------------------------*/
/**
 * Reset the timer with the same interval.
 *
 * This function resets the timer with the same interval that was
 * given to the stimer_set() function. The start point of the interval
 * is the exact time that the timer last expired. Therefore, this
 * function will cause the timer to be stable over time, unlike the
 * stimer_restart() function.
 *
 * \param t A pointer to the timer.
 *
 * \sa stimer_restart()
 */
void
stimer_reset(struct stimer *t)
{
  t->start += t->interval;
}
/*---------------------------------------------------------------------------*/
/**
 * Restart the timer from the current point in time
 *
 * This function restarts a timer with the same interval that was
 * given to the stimer_set() function. The timer will start at the
 * current time.
 *
 * \note A periodic timer will drift if this function is used to reset
 * it. For preioric timers, use the stimer_reset() function instead.
 *
 * \param t A pointer to the timer.
 *
 * \sa stimer_reset()
 */
void
stimer_restart(struct stimer *t)
{
  t->start = clock_seconds();
}
/*---------------------------------------------------------------------------*/
/**
 * Check if a timer has expired.
 *
 * This function tests if a timer has expired and returns true or
 * false depending on its status.
 *
 * \param t A pointer to the timer
 *
 * \return Non-zero if the timer has expired, zero otherwise.
 *
 */
int
stimer_expired(struct stimer *t)
{
  return SCLOCK_GEQ(clock_seconds(), t->start + t->interval);
}
/*---------------------------------------------------------------------------*/
/**
 * The time until the timer expires
 *
 * This function returns the time until the timer expires.
 *
 * \param t A pointer to the timer
 *
 * \return The time until the timer expires
 *
 */
unsigned long
stimer_remaining(struct stimer *t)
{
  return t->start + t->interval - clock_seconds();
}
/*---------------------------------------------------------------------------*/
/**
 * The time elapsed since the timer started
 *
 * This function returns the time elapsed.
 *
 * \param t A pointer to the timer
 *
 * \return The time elapsed since the last start of the timer
 *
 */
unsigned long
stimer_elapsed(struct stimer *t)
{
  return clock_seconds() - t->start;
}

/*---------------------------------------------------------------------------*/

/** @} */