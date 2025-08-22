<script lang="ts">
  import Slider from "./Slider.svelte";

</script>

<div>
  <h1>Xylo synth demo</h1>
  <Slider parameter_id="volume" />
</div>
<div>
  <h2>ADSR</h2>
  <Slider parameter_id="attack" />
  <Slider parameter_id="decay" />
  <Slider parameter_id="sustain" />
  <Slider parameter_id="release" />
</div>