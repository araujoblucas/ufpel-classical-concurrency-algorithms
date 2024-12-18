use std::sync::{Arc, Mutex, Condvar};
use std::thread;
use std::time::Duration;

// Definindo constantes
const MAX_CHAIRS: usize = 2;
const TOTAL_CUSTOMERS: usize = 20;

struct BarberShop {
    waiting_customers: Mutex<usize>,
    customers_left: Mutex<usize>,
    served_customers: Mutex<usize>,
    condvar: Condvar,
}

impl BarberShop {
    fn new() -> Arc<Self> {
        Arc::new(Self {
            waiting_customers: Mutex::new(0),
            customers_left: Mutex::new(0),
            served_customers: Mutex::new(0),
            condvar: Condvar::new(),
        })
    }

    fn customer_enters(&self, id: usize) {
        let mut waiting = self.waiting_customers.lock().unwrap();
        if *waiting < MAX_CHAIRS {
            *waiting += 1;
            let free_chairs = MAX_CHAIRS - *waiting;
            println!("Cliente {} entrou na barbearia. Cadeiras livres: {}", id, free_chairs);
            self.condvar.notify_one();
        } else {
            println!("Cliente {} foi embora: barbearia cheia.", id);
            let mut left = self.customers_left.lock().unwrap();
            *left += 1;
        }
    }

    fn cut_hair(&self) {
        loop {
            let mut waiting = self.waiting_customers.lock().unwrap();

            // Verifica se todos os clientes foram processados
            let total_left = *self.customers_left.lock().unwrap();
            let total_served = *self.served_customers.lock().unwrap();

            if total_left + total_served >= TOTAL_CUSTOMERS {
                println!("Todos os clientes foram processados. Barbearia fechando.");
                return;
            }

            while *waiting == 0 {
                println!("Barbeiro está dormindo...");
                waiting = self.condvar.wait(waiting).unwrap();

                // Verifica novamente se todos os clientes foram processados
                let total_left = *self.customers_left.lock().unwrap();
                let total_served = *self.served_customers.lock().unwrap();
                if total_left + total_served >= TOTAL_CUSTOMERS {
                    println!("Todos os clientes foram processados. Barbearia fechando.");
                    return;
                }
            }

            // Atende o cliente
            *waiting -= 1;
            println!("Barbeiro está cortando cabelo.");
            thread::sleep(Duration::from_secs(2));
            println!("Corte de cabelo finalizado.");

            // Atualiza número de clientes atendidos
            let mut served = self.served_customers.lock().unwrap();
            *served += 1;
        }
    }
}

fn main() {
    let shop = BarberShop::new();

    // Inicia a thread do barbeiro
    let shop_for_barber = Arc::clone(&shop);
    let barber_handle = thread::spawn(move || {
        shop_for_barber.cut_hair();
    });

    // Cria threads de clientes
    let mut customer_handles = Vec::new();
    for i in 1..=TOTAL_CUSTOMERS {
        let shop_for_customer = Arc::clone(&shop);
        let handle = thread::spawn(move || {
            shop_for_customer.customer_enters(i);
        });
        customer_handles.push(handle);
        thread::sleep(Duration::from_millis(500));
    }

    // Aguarda todas as threads de clientes finalizarem
    for handle in customer_handles {
        handle.join().unwrap();
    }

    // Aguarda a finalização do barbeiro
    barber_handle.join().unwrap();
}
