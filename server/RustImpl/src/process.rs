use super::utils::AbortOnDrop;

pub trait Process {
    fn run(&self) -> AbortOnDrop<Result<(), String>>;
}
