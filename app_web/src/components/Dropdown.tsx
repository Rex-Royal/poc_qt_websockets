type DropdownProps<T extends string> = {
  id: string;
  label: string;
  value: T | undefined;
  options: T[];
  placeholder?: string;
  onChange: (value: T) => void;
};

export function Dropdown<T extends string>({
  id,
  label,
  value,
  options,
  placeholder = "-- Select --",
  onChange,
}: DropdownProps<T>) {
  return (
    <div className="flex flex-col items-center">
      <label htmlFor={id} className="block mb-2 text-sm font-semibold">
        {label}
      </label>
      <select
        id={id}
        value={value ?? ""}
        onChange={(e) => onChange(e.target.value as T)}
        className="p-2 border rounded w-full"
      >
        <option value="">{placeholder}</option>
        {options.map((opt) => (
          <option key={opt} value={opt}>
            {opt}
          </option>
        ))}
      </select>
    </div>
  );
}
