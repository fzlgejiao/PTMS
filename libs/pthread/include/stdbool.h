#ifndef _STDBOOL_H_
#define	_STDBOOL_H_	

#define	__bool_true_false_are_defined	1

#ifndef __cplusplus

#define	false	0
#define	true	1

#define	bool	M_BOOL
#if __STDC_VERSION__ < 199901L && __GNUC__ < 3
typedef	char	M_BOOL;
#endif

#endif /* !__cplusplus */

#endif /* !_STDBOOL_H_ */