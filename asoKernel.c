#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JOSEP SEGARRA"); 
MODULE_DESCRIPTION("PHASE 1"); 
MODULE_VERSION("0.1");

 
static unsigned int gpioLEDA = 20;      
static unsigned int gpioLEDB = 21; 
static unsigned int gpioButton1 = 26;  
static unsigned int gpioButton2 = 19;
static unsigned int gpioButton3 = 13;
static unsigned int gpioButton4 = 6;
static unsigned int irqNumber1;   
static unsigned int irqNumber2;          
static unsigned int irqNumber3;          
static unsigned int irqNumber4;                 
static unsigned int numberPresses1 = 0;  
static unsigned int numberPresses2 = 0;  
static unsigned int numberPresses3 = 0;  
static unsigned int numberPresses4 = 0; 
static unsigned int totalPresses = 0; 
static bool ledAOn = 0;      
static bool ledBOn = 0;              

static irq_handler_t  ebbgpio_irq_handler1(unsigned int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t  ebbgpio_irq_handler2(unsigned int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t  ebbgpio_irq_handler3(unsigned int irq, void *dev_id, struct pt_regs *regs);

static irq_handler_t  ebbgpio_irq_handler4(unsigned int irq, void *dev_id, struct pt_regs *regs); 

static char * envp[] = { "HOME=/", NULL };


static int __init ebbgpio_init(void){
   int result = 0;
   printk(KERN_INFO "asoKernel: Initializing the asoKernel LKM\n");

   if (!gpio_is_valid(gpioLEDA)){
      printk(KERN_INFO "asoKernel: invalid LED GPIO\n");
      return -ENODEV;
   }

   ledAOn = false;
   ledBOn = false;
   gpio_request(gpioLEDA, "sysfs"); 
   gpio_request(gpioLEDB, "sysfs");          
         
   gpio_direction_output(gpioLEDA, ledAOn);   
   gpio_direction_output(gpioLEDB, ledBOn);  

   gpio_export(gpioLEDA, false);             
   gpio_export(gpioLEDB, false);            
                     
   gpio_request(gpioButton1, "sysfs");  
   gpio_request(gpioButton2, "sysfs");       
   gpio_request(gpioButton3, "sysfs");       
   gpio_request(gpioButton4, "sysfs");       
     
   gpio_direction_input(gpioButton1); 
   gpio_direction_input(gpioButton2);        
   gpio_direction_input(gpioButton3);        
   gpio_direction_input(gpioButton4);        
       
   gpio_set_debounce(gpioButton1, 200); 
   gpio_set_debounce(gpioButton2, 200);      
   gpio_set_debounce(gpioButton3, 200);      
   gpio_set_debounce(gpioButton4, 200);      
     
   gpio_export(gpioButton1, false);
   gpio_export(gpioButton2, false);          
   gpio_export(gpioButton3, false);          
   gpio_export(gpioButton4, false);          
          
   printk(KERN_INFO "asoKernel: The button1 state is currently: %d\n", gpio_get_value(gpioButton1));
   printk(KERN_INFO "asoKernel: The button2 state is currently: %d\n", gpio_get_value(gpioButton2));
   printk(KERN_INFO "asoKernel: The button3 state is currently: %d\n", gpio_get_value(gpioButton3));
   printk(KERN_INFO "asoKernel: The button4 state is currently: %d\n", gpio_get_value(gpioButton4));

 
   irqNumber1 = gpio_to_irq(gpioButton1);
   irqNumber2 = gpio_to_irq(gpioButton2);
   irqNumber3 = gpio_to_irq(gpioButton3);
   irqNumber4 = gpio_to_irq(gpioButton4);

   printk(KERN_INFO "asoKernel: The button is mapped to IRQ: %d\n", irqNumber1);
   printk(KERN_INFO "asoKernel: The button is mapped to IRQ: %d\n", irqNumber2);
   printk(KERN_INFO "asoKernel: The button is mapped to IRQ: %d\n", irqNumber3);
   printk(KERN_INFO "asoKernel: The button is mapped to IRQ: %d\n", irqNumber4);

 
   result = request_irq(irqNumber1,             
                        (irq_handler_t) ebbgpio_irq_handler1, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                
 
   printk(KERN_INFO "asoKernel: The interrupt 1 request result is: %d\n", result);

   result = request_irq(irqNumber2,             
                        (irq_handler_t) ebbgpio_irq_handler2, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                
 
   printk(KERN_INFO "asoKernel: The interrupt 2 request result is: %d\n", result);

   result = request_irq(irqNumber3,             
                        (irq_handler_t) ebbgpio_irq_handler3, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                 
 
   printk(KERN_INFO "asoKernel: The interrupt 3 request result is: %d\n", result);

   result = request_irq(irqNumber4,             
                        (irq_handler_t) ebbgpio_irq_handler4, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                 
 
   printk(KERN_INFO "asoKernel: The interrupt 4 request result is: %d\n", result);

   return result;
}

 
static void __exit ebbgpio_exit(void){
   printk(KERN_INFO "asoKernel: The button state is currently: %d\n", gpio_get_value(gpioButton1));
   printk(KERN_INFO "asoKernel: The button 1 was pressed %d times\n", numberPresses1);
   printk(KERN_INFO "asoKernel: The button 2 was pressed %d times\n", numberPresses2);
   printk(KERN_INFO "asoKernel: The button 3 was pressed %d times\n", numberPresses3);
   printk(KERN_INFO "asoKernel: The button 4 was pressed %d times\n", numberPresses4);

   printk(KERN_INFO "asoKernel: The buttons were pressed a total of %d times\n", totalPresses);
   gpio_set_value(gpioLEDA, 0);              
   gpio_unexport(gpioLEDA); 
   gpio_set_value(gpioLEDB, 0);              
   gpio_unexport(gpioLEDB);    

   free_irq(irqNumber1, NULL); 
   free_irq(irqNumber2, NULL);               
   free_irq(irqNumber3, NULL);               
   free_irq(irqNumber4, NULL);               
              
   gpio_unexport(gpioButton1);   
   gpio_unexport(gpioButton2);               
   gpio_unexport(gpioButton3);               
   gpio_unexport(gpioButton4);               
            
   gpio_free(gpioLEDA);   
   gpio_free(gpioLEDB);                      
                   
   gpio_free(gpioButton1); 
   gpio_free(gpioButton2);                   
   gpio_free(gpioButton3);                   
   gpio_free(gpioButton4);                   
                  
   printk(KERN_INFO "asoKernel: Goodbye from the LKM!\n");
}
 

static irq_handler_t ebbgpio_irq_handler1(unsigned int irq, void *dev_id, struct pt_regs *regs){
   static char * argv[] = {"/aso/script1.sh", NULL };
   ledAOn = true;                          
   gpio_set_value(gpioLEDA, ledAOn);          
   printk(KERN_INFO "asoKernel: Interrupt! (button 1 pressed)\n");
   numberPresses1++;  
   totalPresses++;
   call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
   return (irq_handler_t) IRQ_HANDLED;      
}

static irq_handler_t ebbgpio_irq_handler2(unsigned int irq, void *dev_id, struct pt_regs *regs){
   static char * argv1[] = { "/aso/script2.sh", NULL };
   ledAOn = false;                         
   gpio_set_value(gpioLEDA, ledAOn);          
   printk(KERN_INFO "asoKernel: Interrupt! (button 2 pressed)\n");
   numberPresses2++;  
   totalPresses++;                
   call_usermodehelper(argv1[0], argv1, envp, UMH_NO_WAIT);   
   return (irq_handler_t) IRQ_HANDLED;      
}

static irq_handler_t ebbgpio_irq_handler3(unsigned int irq, void *dev_id, struct pt_regs *regs){         
   static char * argv2[] = { "/aso/script3.sh", NULL };   
   ledBOn = true;
   gpio_set_value(gpioLEDB, ledBOn);          
   printk(KERN_INFO "asoKernel: Interrupt! (button 3 pressed)\n");
   numberPresses3++;  
   totalPresses++;                       
   call_usermodehelper(argv2[0], argv2, envp, UMH_NO_WAIT);
   return (irq_handler_t) IRQ_HANDLED;      
}

static irq_handler_t ebbgpio_irq_handler4(unsigned int irq, void *dev_id, struct pt_regs *regs){
   static char * argv3[] = { "/aso/script4.sh", NULL };
   ledBOn = false;                          
   gpio_set_value(gpioLEDB, ledBOn);          
   printk(KERN_INFO "asoKernel: Interrupt! (button 4 pressed)\n");
   numberPresses4++;  
   totalPresses++;                       
   call_usermodehelper(argv3[0], argv3, envp, UMH_NO_WAIT);
   return (irq_handler_t) IRQ_HANDLED;      
}


module_init(ebbgpio_init);
module_exit(ebbgpio_exit);

