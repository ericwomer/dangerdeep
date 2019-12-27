
const char *(*DFTD_glGetString) ( GLenum name );
#define glGetString( a ) (*DFTD_glGetString)( (a) )
void (*DFTD_glGetIntegerv)( GLenum pname, GLint *params );
#define glGetIntegerv( a, b ) (*DFTD_glGetIntegerv)( (a), (b) )


