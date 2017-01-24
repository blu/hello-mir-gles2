#ifndef gles_gl_mapping_H__
#define gles_gl_mapping_H__

#if PLATFORM_GL == 0
#error platform is not desktop GL
#endif

#if !defined(GL_DEPTH_STENCIL_OES)
#define GL_DEPTH_STENCIL_OES GL_DEPTH_STENCIL
#endif

#if !defined(GL_UNSIGNED_INT_24_8_OES)
#define GL_UNSIGNED_INT_24_8_OES GL_UNSIGNED_INT_24_8
#endif

#if !defined(GL_RGB565)
#define GL_RGB565 GL_RGB5
#endif

// desktop GL has had VAOs in GL Core since version 3.0
#if PLATFORM_GL_OES_vertex_array_object == 0
	#define PLATFORM_GL_OES_vertex_array_object 1
#endif
#define glBindVertexArrayOES    glBindVertexArray
#define glDeleteVertexArraysOES glDeleteVertexArrays
#define glGenVertexArraysOES    glGenVertexArrays
#define glIsVertexArrayOES      glIsVertexArray

#if GL_ARB_debug_output != 0
#define glDebugMessageControlKHR  glDebugMessageControlARB
#define glDebugMessageInsertKHR   glDebugMessageInsertARB
#define glDebugMessageCallbackKHR glDebugMessageCallbackARB
#define glGetDebugMessageLogKHR   glGetDebugMessageLogARB
#define glPushDebugGroupKHR       ((void*) 0) // no GL equivalent
#define glPopDebugGroupKHR        ((void*) 0) // no GL equivalent
#define glObjectLabelKHR          ((void*) 0) // no GL equivalent
#define glGetObjectLabelKHR       ((void*) 0) // no GL equivalent
#define glObjectPtrLabelKHR       ((void*) 0) // no GL equivalent
#define glGetObjectPtrLabelKHR    ((void*) 0) // no GL equivalent
#define glGetPointervKHR          glGetPointerv

#define GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR         GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB
#define GL_MAX_DEBUG_MESSAGE_LENGTH_KHR         GL_MAX_DEBUG_MESSAGE_LENGTH_ARB
#define GL_MAX_DEBUG_LOGGED_MESSAGES_KHR        GL_MAX_DEBUG_LOGGED_MESSAGES_ARB
#define GL_DEBUG_LOGGED_MESSAGES_KHR            GL_DEBUG_LOGGED_MESSAGES_ARB
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB
#define GL_DEBUG_CALLBACK_FUNCTION_KHR          GL_DEBUG_CALLBACK_FUNCTION_ARB
#define GL_DEBUG_CALLBACK_USER_PARAM_KHR        GL_DEBUG_CALLBACK_USER_PARAM_ARB
#define GL_DEBUG_SOURCE_API_KHR                 GL_DEBUG_SOURCE_API_ARB
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR       GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB
#define GL_DEBUG_SOURCE_SHADER_COMPILER_KHR     GL_DEBUG_SOURCE_SHADER_COMPILER_ARB
#define GL_DEBUG_SOURCE_THIRD_PARTY_KHR         GL_DEBUG_SOURCE_THIRD_PARTY_ARB
#define GL_DEBUG_SOURCE_APPLICATION_KHR         GL_DEBUG_SOURCE_APPLICATION_ARB
#define GL_DEBUG_SOURCE_OTHER_KHR               GL_DEBUG_SOURCE_OTHER_ARB
#define GL_DEBUG_TYPE_ERROR_KHR                 GL_DEBUG_TYPE_ERROR_ARB
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR   GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR    GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB
#define GL_DEBUG_TYPE_PORTABILITY_KHR           GL_DEBUG_TYPE_PORTABILITY_ARB
#define GL_DEBUG_TYPE_PERFORMANCE_KHR           GL_DEBUG_TYPE_PERFORMANCE_ARB
#define GL_DEBUG_TYPE_OTHER_KHR                 GL_DEBUG_TYPE_OTHER_ARB
#define GL_DEBUG_SEVERITY_HIGH_KHR              GL_DEBUG_SEVERITY_HIGH_ARB
#define GL_DEBUG_SEVERITY_MEDIUM_KHR            GL_DEBUG_SEVERITY_MEDIUM_ARB
#define GL_DEBUG_SEVERITY_LOW_KHR               GL_DEBUG_SEVERITY_LOW_ARB

#endif
#endif // gles_gl_mapping_H__
