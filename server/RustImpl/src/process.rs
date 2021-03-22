use super::util::AbortOnDrop;

pub trait Process {
    fn run(&self) -> AbortOnDrop<Result<(), String>>;
}
