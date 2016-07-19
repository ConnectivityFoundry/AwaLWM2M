
![](images/img.png)

----


# Awa LightweightM2M.


## Coding style guide.

It's important that coding style remains consistent throughout the project to ensure that all contributors and project administrators are able to easily decipher code blocks, comments, syntax etc. 
Always consider that another contributor may one day continue to build on your submissions. Make it easy for them by adopting the style outlined below.

### General layout.

The sample code below illustrates the preferred use of indentation, bracing and white space:

````
AwaDatetime AwaObject_GetArrayDateTimeProperty(AwaObject self, int propertyIndex, int itemIndex)
{
    AwaDatetime result = 0;
    AwaArray awaArray = AwaObject_GetArrayProperty(self, propertyIndex);
    if (awaArray)
    {
        if ((itemIndex < awaArray->Used) && (itemIndex >= 0))
        {
            result = *(Awa_Datetime*)awaArray->Items[itemIndex];
        }
    }
    return result;
}
````
**Note:**  
  * There is no white space before the parenthesis in a FunctionName(), but there *is* a space before the parenthesis in conditional statements: if (), for (), while () etc.
  * Opening braces are on the line following the closing parenthesis of the function or condition to which they relate.
  * The maximum recommended line length is 160 characters. For readability, lines longer than 160 characters may be wrapped.
  * Tabs should be replaced with 4 space characters.


### Naming conventions.

* Use meaningful names.  
* Names must not be too abbreviated or cryptic. For example use *controlState* not *ctl_st*.  
* Underscores within a name are discouraged.
* Function names, typedefs, structures etc. should be mixed case and should start with upper case. For example use *GetQueueCount()*, rather than *get_queue_count()*.  
* Local variable names (for function parameters or variables declared within a function) should be mixed case *but should start with lowercase*. For example use *logCount* rather than *log_count*.  
* Static variables within a module should use an underscore prefix and use pascal case (e.g. _RequestQueue). This makes it obvious where the variables are declared when reading code within each module.  
* Global variables (if genuinely required), should use the prefix "g_" and use pascal case (e.g. g_CurrentUser). This helps to warn a developer that changing or using the variable can have far reaching consequences (for thread or interrupt safety etc).  
* Pointers should be given meaningful names *without* the use of the prefix  *p* or *ptr_* . For example use **newSettings*, not **ptr_new_settings* or **pNewSettings*.  
* Do not use the suffix *_t* in structure type definition names. 

 Sample structure definition:
````
typedef struct
{
    int RequestID;
    AwaSemaphore Semphore;
    void * Buffer;
    size_t BufferSize;
    bool Success;
} RequestContext;
````

**Notes.** 

* A structure type definition doesn't usually need both a structure name and typedef name (a struct name is not given in the above example). One possible exception to this rule is where the typedef name is declaring a structure-pointer type.  
* The use of suffix *_e* in enums is discouraged. 
* To avoid namespace problems in 'C' each value should be prefixed by the enum name. For example:
````
typedef enum
{
    AwaColourGreen,
    AwaColourBlue
} AwaColour;
````

* Defined values should normally be all uppercase with underscore separators. For example: 

````    #define WAIT_TIMEOUT 100 ````
 
* Module names should be all lowercase with underscore separators.


### Type definitions.

Awa LWM2M makes uses of the type definitions defined in ````<stdint.h>```` and ````<stdbool.h>````.

### Structures, unions and bitfields.

While unions and bitfields are valid 'C' constructs, they negatively impact readibility and portability.
Bitfields also have a code size and performance overhead due to the need to be parsed for read/write operations. In multi-tasking or interrupt driven systems bitfields are also vulnerable to corruption unless special care is taken. Since the small saving in data space is outweighed by the risk and overheads, the use of bitfields and unions is discouraged.

Structure field names should use Pascal casing (e.g. ThisPerson.Age).

### Header files.

All public functions, variables and definitions must be declared in a header file. Most .c modules should have a corresponding .h file to be included (both within the module and elsewhere). It is poor form to make the compiler try to guess how to resolve undeclared function calls in other modules.

### Code commenting.
Single line comments should use *//*. For example:
````
void MyFunction(void)
{
    ...
    // Send request headers
    SendHeader(headerName1, headerValue1);
    SendHeader(headerName2, headerValue2);
 
     // Send request body
    SendBody(body, bodyLength);    
}
````

Similarly, multi-line comments within a function should be formatted like this:
````
void AwaTask(void *parameters)
{
    AwaInit();
    while (true)
    {
        // Wait for messages from libawa until something arrives in the queue -
        // this task will block indefinitely provided INCLUDE_vTaskSuspend is set
        // to 1 in FreeRTOSConfig.h
        xQueueReceive(g_TxQueue, &recvdReq, PORT_MAX_DELAY);
 
        // Handle received message...
    }
}
````

The use of function headers is optional. While well named functions often don't need a header to describe what they do, it may help other developers to explain what a function does or why,  or how other parts of the code are affected. Where a function header is provided it should be formatted like this:
````
/**
* Get the next entry from the processing queue. 
* This should be polled at least every 100ms to avoid blocking user requests
* for too long. Note: need to make this event driven.
*/
AwaString GetNextEntryFromProcessingQueue()
{
    // Do something
}
 ````
 
### Good coding practise.
* Ideally functions should have a single exit point. The use of multiple return points is dangerous and makes it harder to read the code. Always consider what will happen if someone needs to append code to your function.

* Where possible definitions should be used rather than raw values, e.g. use *AllocMessageBuf(MAX_MESSAGE_SIZE)* rather than  *AllocMessagBuf(1024)*. This ensures that all code sharing the same defined value will still work if the 'magic' value is updated.

* Recursion - **Do not use**. Embedded code has limited stack space. 

----
----
