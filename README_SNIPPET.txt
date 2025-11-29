Instrucciones rápidas (en tu VM):

1) Crea la estructura de carpetas:
   mkdir -p homework3/{src,include,tests,workloads}
   cd homework3

2) Crea archivos con los contenidos provistos (copiar/pegar). Por ejemplo:
   nano include/scheduler.h   (pegar el contenido)
   nano include/algorithms.h
   ...

3) Compilar:
   make

4) Ejecutar:
   ./scheduler workloads/workload1.txt fifo
   ./scheduler workloads/workload1.txt sjf
   ./scheduler workloads/workload1.txt stcf
   ./scheduler workloads/workload1.txt rr 3
   ./scheduler workloads/workload1.txt mlfq 3 2,4,8 50

   Nota: el ejecutable preguntará si quieres lanzar la GUI (ncurses). Teclea 'y' para ver la vista.

5) Ejecutar tests unitarios rápidos:
   ./build/test_fifo
   ./build/test_rr
   ...

Observaciones:
- El proyecto está pensado para ser legible y fácil de extender.
- Las funciones de scheduling actualizan las estructuras de procesos con start/completion.
- El Gantt en ncurses es aproximado (usa bloques de caracteres).
- Para generar un reporte en Markdown, puedes usar la función generate_report (de ejemplo).
