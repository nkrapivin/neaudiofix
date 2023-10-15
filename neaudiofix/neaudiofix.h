// Приведенный ниже блок ifdef — это стандартный метод создания макросов, упрощающий процедуру
// экспорта из библиотек DLL. Все файлы данной DLL скомпилированы с использованием символа NEAUDIOFIX_EXPORTS
// Символ, определенный в командной строке. Этот символ не должен быть определен в каком-либо проекте,
// использующем данную DLL. Благодаря этому любой другой проект, исходные файлы которого включают данный файл, видит
// функции NEAUDIOFIX_API как импортированные из DLL, тогда как данная DLL видит символы,
// определяемые данным макросом, как экспортированные.
#ifdef NEAUDIOFIX_EXPORTS
#define NEAUDIOFIX_API __declspec(dllexport)
#else
#define NEAUDIOFIX_API __declspec(dllimport)
#endif

#define NEAUDIOFIX_CALL __cdecl

#ifdef __cplusplus
#define NEAUDIOFIX_EXTERN extern "C"
#else
#define NEAUDIOFIX_EXTERN /* no-op */
#endif
