#ifndef LC_ADDRLABELS_H_
#define LC_ADDRLABELS_H_

/** \hideinitializer */
typedef void * lc_t;

#define LC_INIT(s) s = NULL


#define LC_RESUME(s)                            \
  do {                                          \
    if(s != NULL) {                             \
      goto *s;                                  \
    }                                           \
  } while(0)

#define LC_SET(s)                               \
  do { ({ __label__ resume; resume: (s) = &&resume; }); }while(0)

#define LC_END(s)

#endif /* LC_ADDRLABELS_H_ */
/** @} */