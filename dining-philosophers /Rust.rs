use std::sync::{Arc, Mutex, Condvar};
use std::thread;
use std::time::Duration;

struct Philosopher {
    name: String,
    left_fork: usize,
    right_fork: usize,
}

impl Philosopher {
    fn new(name: &str, left_fork: usize, right_fork: usize) -> Philosopher {
        Philosopher {
            name: name.to_string(),
            left_fork,
            right_fork,
        }
    }

    fn dine(&self, table: Arc<Table>) {
        loop {
            self.think();
            self.eat(&table);
        }
    }

    fn think(&self) {
        println!("{} está pensando...", self.name);
        thread::sleep(Duration::from_millis(1000));
    }

    fn eat(&self, table: &Table) {
        let (left, right) = (self.left_fork, self.right_fork);

        // Bloqueia os garfos usando um lock nos mutexes, com uma ordem para evitar deadlocks
        {
            let mut forks = table.forks.lock().unwrap();
            while forks[left] || forks[right] {
                forks = table.condvar.wait(forks).unwrap();
            }
            forks[left] = true;
            forks[right] = true;
        } // Os garfos são bloqueados aqui

        println!("{} está comendo...", self.name);
        thread::sleep(Duration::from_millis(1000));

        // Libera os garfos
        {
            let mut forks = table.forks.lock().unwrap();
            forks[left] = false;
            forks[right] = false;
            table.condvar.notify_all();
        } // Os garfos são desbloqueados aqui
        println!("{} terminou de comer...", self.name);

    }
}

struct Table {
    forks: Mutex<Vec<bool>>,
    condvar: Condvar,
}

fn main() {
    let table = Arc::new(Table {
        forks: Mutex::new(vec![false; 5]),
        condvar: Condvar::new(),
    });

    let philosophers = vec![
        Philosopher::new("Filósofo 1", 0, 1),
        Philosopher::new("Filósofo 2", 1, 2),
        Philosopher::new("Filósofo 3", 2, 3),
        Philosopher::new("Filósofo 4", 3, 4),
        Philosopher::new("Filósofo 5", 4, 0),
    ];

    let handles: Vec<_> = philosophers.into_iter().map(|p| {
        let table = table.clone();
        thread::spawn(move || {
            p.dine(table);
        })
    }).collect();

    for handle in handles {
        handle.join().unwrap();
    }
}
