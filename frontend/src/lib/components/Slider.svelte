<script lang="ts">
  import { onMount } from "svelte";
  import { audioWorkletNode } from "../../main";

  let { parameter_id }: { parameter_id: string } = $props();

  let inputComponent: HTMLInputElement;
  let valueComponent: HTMLSpanElement;

  const parameter = audioWorkletNode!.parameters.get(Module!.parameterIdToIndex(parameter_id))!;
  const parameterView = Module!.getParameter(parameter_id);

  function onChange() {
    const normalizedValue = parseFloat(inputComponent.value);
    const value = parameterView.convertFromNormalizedValue(normalizedValue);
    valueComponent.innerText = parameterView.getValueToString(value, 2);
    parameter.value = normalizedValue;
  }

  onMount(() => {
    inputComponent.oninput = () => { onChange(); };
    onChange();
  });
</script>

<div>
  <span>Choose {parameter_id}: </span>
  <input bind:this={inputComponent} type="range" step="any" min="{parameter.minValue}" max="{parameter.maxValue}" value="{parameter.value}" style="width: 300px">
  <span bind:this={valueComponent}>{parameter.value}</span>
</div>